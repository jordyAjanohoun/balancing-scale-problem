# balancing-scale-problem

## How to build
```bash
g++ -std=c++20 main.cpp -o balance
```

## Usage
```bash
./balance <input_file_path>
```

## Example
```bash
./balance test_6.txt
```

## Assumptions
These are the assumptions I've made and consider reasonable:
- Masses are `unsigned long long`. A mass cannot be negative. If a mass is negative or overflows, an exception will be thrown.
- Empty lines are valid don't invalidate the input.
- A scale name cannot contain whitespaces. The code needs to be updated (small change) if we want to support whitespaces in scale names.
- I don't assume that the nodes in the input file appear from root to leaves.
- Each scale must have an entry (row) in the input file, even if there is no mass or scale on the pans. Example: A1,0,0
- The total mass of the final balanced tree doesn't exceed the upper limit of `unsigned long long`. The program doesn't check or verify that. We could if needed.

## Notes
- I manually tested using the test files in this repo. Using a test framework like GTest or Catch2 and setting up automated tests (CI/CD) is better for real world projects. 
- The runtime complexity is O(N) where N is the number of nodes because we traverse the tree 2 times. Once to parse the input file and store the tree as a map, and a second time to compute the masses we need to balance the tree.
- The memory complexity is also O(N) because the data structures I use store no more than N elements where N is the number of nodes in the tree.
- To keep things simple, I'm handling errors by throwing. I appreciate that there are other error handling approaches.
- You might notice that the second commit is not attributed to my account/profile. This is because the email in my git config was not set. I've set it when I realized.   
