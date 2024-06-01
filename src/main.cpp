#include <iostream>
#include <unordered_set>
#include <vector>
#include <fstream>
#include <unordered_map>

#define unlikely(expr) __builtin_expect(!!(expr), 0)
#define likely(expr) __builtin_expect(!!(expr), 1)

#include <vector>
#include <unordered_map>

void printProgressBar(int progress, int total)
{
    int barWidth = 70;
    float percentage = (float)progress / total;
    int pos = barWidth * percentage;

    std::cout << "[";
    for (int i = 0; i < barWidth; ++i)
    {
        if (i < pos)
            std::cout << "=";
        else if (i == pos)
            std::cout << ">";
        else
            std::cout << " ";
    }
    std::cout << "] " << int(percentage * 100.0) << " %\r";
    std::cout.flush();
}

class Node
{
public:
    std::unordered_map<
        std::string, std::vector<Node *>>
        children;
    std::string path;
    uint32_t rcounter;
    uint32_t scounter;

    Node() : rcounter(0), scounter(0) {}
};

Node build_suffix_tree(std::vector<std::string> words)
{
    std::unordered_map<std::string, Node *> shortcuts;
    Node root = Node();
    root.path = "1";

    int index = 0;
    int num_words = words.size();
    for (std::string s : words)
    {
        index++;
        if (index % 1000 == 0)
            printProgressBar(index, num_words);

        std::unordered_set<std::string> paths;

        for (int i = 0; i < s.size(); i++)
        {
            std::string suffix = s.substr(i, s.size()) + "$";
            bool is_full_string = i == 0;

            // add suffix
            Node *current = &root;
            std::vector<Node *> ancestors = std::vector<Node *>();
            for (int j = 0; j < suffix.size(); j++)
            {
                std::string c = suffix.substr(j, 1);
                Node *next = nullptr;

                std::string char_string = std::string{c};
                if (current->children.find(char_string) == current->children.end())
                {
                    current->children[c] = std::vector<Node *>();
                    next = new Node();
                    next->path = current->path + "." + std::to_string(current->children.size());
                    shortcuts[next->path] = next;
                    current->children[c].push_back(next);
                }
                else
                {
                    next = current->children[c][0];
                }

                current = next;

                if (likely(!(paths.find(next->path) == paths.end())))
                {
                    paths.insert(next->path);

                    ++next->rcounter;

                    if (unlikely(is_full_string))
                    {
                        ++next->scounter;
                    }
                }

                if (likely(j != suffix.size() - 1))
                {
                    char next_char = suffix[j + 1];
                    bool is_next_char_terminal = next_char == '$';
                    if (!is_next_char_terminal)
                    {
                        if (j > 0)
                        {
                            Node *parent = ancestors[ancestors.size() - 1];

                            std::string dashed_path = "_";
                            dashed_path += next_char;

                            if (parent->children.find(dashed_path) == parent->children.end())
                            {
                                parent->children[dashed_path] = std::vector<Node *>();
                            }

                            parent->children[dashed_path].push_back(next);
                        }

                        for (Node *ancestor : ancestors)
                        {
                            std::string percent_path = "%";
                            percent_path += next_char;

                            if (ancestor->children.find(percent_path) == ancestor->children.end())
                            {
                                ancestor->children[percent_path] = std::vector<Node *>();
                            }

                            ancestor->children[percent_path].push_back(next);
                        }
                    }
                }

                ancestors.push_back(current);
                current = next;
            }
        }
    }

    return root;
}

#include <stack>

std::unordered_map<std::string, Node *> traverse_iterative(Node *root, std::string pattern)
{
    std::unordered_map<std::string, Node *> result;

    int index = 0;
    if (pattern[0] == '%')
        index = 1;

    pattern += "$";
    int len_pattern = pattern.size();

    std::stack<std::pair<Node *, int>> stack;

    stack.push(std::make_pair(root, index));

    while (!stack.empty())
    {
        std::pair<Node *, int> current = stack.top();
        stack.pop();

        Node *node = current.first;
        const int index = current.second;

        if (index > len_pattern)
            continue;

        if (index != 0 && pattern[0] != '%' && node->scounter == 0)
            continue;

        const char c = pattern[index];
        const int next_index = index + 1;

        if (c == '$' && node->children.find("$") != node->children.end())
        {
            auto end_node = node->children.at("$")[0];
            result[end_node->path] = end_node;
        }

        if (c == '%')
        {
            if (next_index < len_pattern && pattern[next_index] == '$')
            {
                result[node->path] = node;
            }
            else
            {
                char label = pattern[next_index];
                std::string percent_path = "%";
                percent_path += label;
                std::string dashed_path = "_";
                dashed_path += label;

                if (node->children.find(percent_path) != node->children.end())
                {
                    for (Node *child : node->children.at(percent_path))
                    {
                        stack.push(std::make_pair(child, next_index));
                    }
                }
                if (node->children.find(dashed_path) != node->children.end())
                {
                    for (Node *child : node->children.at(dashed_path))
                    {
                        stack.push(std::make_pair(child, next_index));
                    }
                }
                if (node->children.find(std::to_string(label)) != node->children.end())
                {
                    Node *next_node = node->children.at(dashed_path)[0];
                    stack.push(std::make_pair(next_node, next_index + 1));
                }
            }
        }
        else if (c == '_')
        {
            std::string next_string = "_";
            next_string += pattern[next_index];
            if (node->children.find(next_string) != node->children.end())
            {
                for (Node *child : node->children.at(next_string))
                {
                    stack.push(std::make_pair(child, next_index + 1));
                }
            }
        }
        else
        {
            std::string char_string = std::string(1, c);
            if (auto search = node->children.find(char_string); search != node->children.end())
            {
                Node *n = search->second[0];
                stack.push(std::make_pair(n, next_index));
            }
        }
    }
}

int main(int argc, char **argv)
{
    std::string file_path = argv[1];

    std::vector<std::string> words;

    std::ifstream myfile(file_path);
    std::string line;

    int index = 0;
    while (std::getline(myfile, line))
    {
        if (++index == 100)
            break;

        words.push_back(line);
    }

    myfile.close();

    std::cout << argv[2] << std::endl;

    auto root = build_suffix_tree(words);

    // %S%N%A%A%KRAM%
    std::string pattern = argv[2];

    auto result = traverse_iterative(&root, pattern);
    std::cout << "Result: " << result.size() << std::endl;

    return 0;
}