# This includes the argument parser, error output, sentence-transformers classes and helper functions
import argparse
import sys
from sentence_transformers import SentenceTransformer, util

# This function loads the sentence-transformers model
def load_model(model_name):

    # Here I write diagnostics to stderr so that stdout is left with only the number text - the same principle already used by florence_caption.py,
    # so that the C++ side can safely read just one line with the result.
    print("Semantic model: " + model_name, file=sys.stderr)

    model = SentenceTransformer(model_name)  # Load the model

    return model    # Return the loaded model

# This function computes the semantic similarity between two texts
def compute_similarity(model, text1, text2):
    embeddings = model.encode([text1, text2])                   # Here both texts are converted into vectors (embeddings)

    similarity = util.cos_sim(embeddings[0], embeddings[1])     # Then the cosine similarity between the two vectors is computed

    score = float(similarity[0][0])                             # Then I take the numeric value from the result tensor

    # If the result is somehow negative (rare, but theoretically possible), I replace it with 0, since a negative semantic similarity is not meaningful on the C++ side
    if score < 0.0:
        score = 0.0     # Then set it to 0.

    return score    # And return the similarity value

# This is the main function
def main():
    parser = argparse.ArgumentParser()                              # Here I create the argument parser
    parser.add_argument("text1")                                    # Here I add the first text
    parser.add_argument("text2")                                    # Here I add the second text
    parser.add_argument("--model", default="all-MiniLM-L6-v2")      # Here I add the model name
    args = parser.parse_args()                                      # Here I parse the arguments
    model = load_model(args.model)                                  # Here I load the model
    similarity = compute_similarity(model, args.text1, args.text2)  # Similarity computation

    print(similarity)   #  Here only the number is output for the C++ program

# Program launch
if __name__ == "__main__":
    main()      # Call the main function
