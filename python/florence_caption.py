# This includes the argument parser, error output, image opening, PyTorch and Hugging Face Florence-2 loading
import argparse
import sys
from PIL import Image
import torch
from transformers import AutoProcessor, AutoModelForCausalLM

# This function automatically chooses the device
def choose_device():

    if torch.cuda.is_available():   # If CUDA is available in the PyTorch environment, use the GPU
        return "cuda"               # Return the CUDA device
    return "cpu"                    # If CUDA is not available, use the CPU.

# This function loads the Florence-2 model
def load_model(model_name):

    device = choose_device()    # Choose the device

    # If CUDA is available, use float16 to reduce GPU memory
    if device == "cuda":
        dtype = torch.float16       # Prepare the float16 type

    # If CPU is used, use float32
    else:
        dtype = torch.float32   # Prepare the float32 type

    # Here I write diagnostics to stderr so that stdout is left with only the caption or region count text
    # The C++ side now does NOT merge stderr back into stdout (see Florence.cpp), so this line
    # should no longer end up in the measured caption.
    print("Florence device: " + device, file=sys.stderr)

    # Load the Florence processor
    processor = AutoProcessor.from_pretrained(model_name, trust_remote_code=True)

    # Load the Florence model
    model = AutoModelForCausalLM.from_pretrained(
        model_name,
        trust_remote_code=True,
        torch_dtype=dtype
    )

    model = model.to(device)     # Move the model to the chosen device

    model.eval()    # Turn on inference mode

    return processor, model, device, dtype      # Return everything needed for generation

# This function runs the Florence-2 model with the specified task prompt and returns the raw result
# It is used both by regular caption generation (<CAPTION>) and region detection (<DENSE_REGION_CAPTION>)
def run_florence_task(processor, model, device, dtype, image_path, task):

    image = Image.open(image_path).convert("RGB")                       # Open the image in RGB mode
    inputs = processor(text=task, images=image, return_tensors="pt")    # Prepare the input for the model with the given task prompt
    input_ids = inputs["input_ids"].to(device)                          # Move input_ids to the device

    # If CUDA is available, convert pixel_values to float16
    if device == "cuda":
        pixel_values = inputs["pixel_values"].to(device, dtype=dtype)   # Here I move the image tensor to CUDA

    # If CPU is used, keep the normal type
    else:
        pixel_values = inputs["pixel_values"].to(device)    # Here I move the image tensor to the CPU

    # Gradients are disabled here since the model is not being trained
    with torch.no_grad():

        # Here I generate the text tokens. max_new_tokens has been raised from 64 to 1024, compared to the previous version, because the region 
        # detection task (<DENSE_REGION_CAPTION>) needs far more tokens than a regular short caption - each detected region takes up
        # several tokens. For the regular <CAPTION> task this changes nothing, since the model stops itself with an EOS token when the caption is ready.
        generated_ids = model.generate(
            input_ids=input_ids,
            pixel_values=pixel_values,
            max_new_tokens=1024,
            num_beams=3
        )

    # Here I convert the tokens to text
    generated_text = processor.batch_decode(
        generated_ids,
        skip_special_tokens=False
    )[0]

    # Here Florence post-processing returns a structured result depending on the task
    result = processor.post_process_generation(
        generated_text,
        task=task,
        image_size=(image.width, image.height)
    )

    return result   # And then return the whole result dictionary, the caller will take the part it needs

# This function generates the image caption using the <CAPTION> task
def generate_caption(processor, model, device, dtype, image_path):
    prompt = "<CAPTION>"    # This is the Florence caption task prompt

    result = run_florence_task(processor, model, device, dtype, image_path, prompt) # Here Florence is run with the caption task

    return result[prompt]   # Return the caption text

# This function counts how many regions (areas) Florence found in the image, using the region detection task.
# It is used by the ImagePurpose class on the C++ side as a simple image complexity indicator.
def count_regions(processor, model, device, dtype, image_path, task):
    result = run_florence_task(processor, model, device, dtype, image_path, task)   # Here Florence is run with the given region detection task

    # If the result does not contain the expected task key, I consider there to be no regions
    if task not in result:
        return 0    # Return 0.

    # If the result does not contain the "bboxes" key, I consider there to be no regions
    if "bboxes" not in result[task]:
        return 0    # Return 0.

    return len(result[task]["bboxes"])  # Return the number of regions found

# This is the main function
def main():
    parser = argparse.ArgumentParser()                                  # Here I create the argument parser
    parser.add_argument("image_path")                                   # Here I add the image path
    parser.add_argument("--model", default="microsoft/Florence-2-base") # And here I add the model name

    # Here I add the task flag - by default a regular caption, but region detection can also be requested.
    # IMPORTANT NOTE: here I use simple words ("caption" / "regions"), NOT the actual internal Florence-2 task tokens ("<CAPTION>" / "<DENSE_REGION_CAPTION>"),
    # even though those are the real tokens the model needs. In the older versions I used "<DENSE_REGION_CAPTION>" directly as the command-line parameter,
    # and the C++ side passed it on to the cmd.exe command - it turned out that Windows cmd.exe interprets the "<" and ">" characters as file redirection signs even inside quotes,
    # and the command always failed with the error "The system cannot find the file specified" (cmd.exe tried to open a file named "DENSE_REGION_CAPTION" for input redirection).
    # The solution was: use simple words without special characters on the command line, and here, on the Python side, convert them into the real Florence-2 tokens.
    parser.add_argument("--task", default="caption")

    args = parser.parse_args()  # Parse the arguments
    processor, model, device, dtype = load_model(args.model)     # Load the model

    # If the task is a regular caption, I output only the caption text
    if args.task == "caption":
        caption = generate_caption(processor, model, device, dtype, args.image_path)    # Generate the caption
        print(caption)  # Output only the caption text for the C++ program
        return

    # If the task is region detection, I convert the simple word into the real Florence-2 token
    if args.task == "regions":
        regionCount = count_regions(processor, model, device, dtype, args.image_path, "<DENSE_REGION_CAPTION>") # Here I call region counting with the real token
        print(regionCount)  # Here I output only the region count for the C++ program
        return

    # If the task is not recognized, an error is output to stderr and I exit with a failure code
    print("ERROR: unknown --task value: " + args.task, file=sys.stderr)
    sys.exit(1)

# Program launch
if __name__ == "__main__":
    main()  # Call the main program
