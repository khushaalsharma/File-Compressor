#include <iostream>
#include <fstream>
#include <queue>
#include <unordered_map>
#include <string>
#include <vector>
#include <bitset>

using namespace std;

class huffTree
{
private:
    int freq;
    string token;

public:
    huffTree *left;
    huffTree *right;

    huffTree(int val, string ch)
    {
        this->freq = val;
        this->token = ch;
        this->left = NULL;
        this->right = NULL;
    }

    huffTree(int val, string ch, huffTree *leftNode, huffTree *rightNode)
    {
        this->freq = val;
        this->token = ch;
        this->left = leftNode;
        this->right = rightNode;
    }

    int getFrequency()
    {
        return this->freq;
    }

    string getToken()
    {
        return this->token;
    }
};

class Huffman
{
private:
    struct cmp
    {
        bool operator()(huffTree *left, huffTree *right)
        {
            return left->getFrequency() > right->getFrequency();
        }
    }; // comparator function for minheap of tree nodes

    unordered_map<string, int> charFreq;
    unordered_map<string, string> huffTable;
    unordered_map<string, string> reverseHuffTable; // for decoding
    huffTree *root;
    priority_queue<huffTree *, vector<huffTree *>, cmp> minheap;

    void buildCharFreqTable(string &data)
    {
        for (char ch : data)
        {
            string ind(1, ch);
            charFreq[ind]++;
        } // building character-frequency table
    }

    void buildTree()
    {
        for (auto itr : charFreq)
        {
            huffTree *node = new huffTree(itr.second, itr.first);
            minheap.push(node);
        } // adding all the char-freq nodes in the minheap

        while (minheap.size() != 1)
        {
            huffTree *left = minheap.top();
            minheap.pop();
            huffTree *right = minheap.top();
            minheap.pop();

            huffTree *newNode = new huffTree(left->getFrequency() + right->getFrequency(), left->getToken() + right->getToken(), left, right);
            minheap.push(newNode);
        } // building the huffman tree

        root = minheap.top();
    }

    void buildCodes(huffTree *root, string encode)
    {
        if (!root)
        {
            return;
        }
        if (!root->left && !root->right)
        {
            huffTable[root->getToken()] = encode;
            reverseHuffTable[encode] = root->getToken(); // for decoding
            return;
        }

        buildCodes(root->left, encode + "0");
        buildCodes(root->right, encode + "1");
    }

    void printTree(huffTree *node)
    {
        if (node == NULL)
        {
            cout << -1 << " ";
            return;
        }

        cout << node->getFrequency() << " ";

        printTree(node->left);
        printTree(node->right);
    }

    void deleteTree(huffTree *node)
    {
        if (node == NULL)
        {
            return;
        }

        deleteTree(node->left);
        deleteTree(node->right);

        delete node;
    }

    void writeCompressedFile(string &fileName, string &ogData)
    {
        ofstream compressedfile(fileName, ios::binary);
        if (!compressedfile.is_open())
        {
            cerr << "Failed to open file for writing: " << fileName << endl;
            return;
        }

        string encodeData = "";

        for (char ch : ogData)
        {
            string ind(1, ch);
            encodeData += huffTable[ind];
        }

        size_t paddedLength = (encodeData.size() + 7) / 8 * 8;
        string paddedBitString = encodeData;
        paddedBitString.append(paddedLength - encodeData.size(), '0');
        for (size_t i = 0; i < paddedBitString.size(); i += 8)
        {
            bitset<8> byte(paddedBitString.substr(i, 8));
            unsigned char byteValue = static_cast<unsigned char>(byte.to_ulong());
            compressedfile.write(reinterpret_cast<const char *>(&byteValue), sizeof(byteValue));
        } // this loop will convert a string of 8 bits into byte and write it in the compressedFile

        compressedfile.close();
    }

    void readCompressedFile(string &compFile, string &decompFile)
    {
        ifstream compressedFile(compFile, ios::binary);
        if (!compressedFile.is_open())
        {
            cerr << "Failed to open file for reading: " << compFile << endl;
            return;
        }

        vector<unsigned char> compressedData((istreambuf_iterator<char>(compressedFile)), {});

        compressedFile.close();
        string bitString;
        string decodedData = "";

        for (unsigned char byte : compressedData)
        {
            bitset<8> bits(byte);
            bitString += bits.to_string();
        }

        string currentBitString = "";
        for (char bit : bitString)
        {
            currentBitString += bit;
            if (reverseHuffTable.find(currentBitString) != reverseHuffTable.end())
            {
                decodedData += reverseHuffTable[currentBitString];
                currentBitString = "";
            }
        }

        ofstream decompressFile(decompFile, ios::binary);
        if (!decompressFile.is_open())
        {
            cerr << "Failed to open file for writing: " << decompFile << endl;
            return;
        }

        decompressFile << decodedData;
        decompressFile.close();
    }

public:
    void beginCompression(string &ogFile, string &compFile)
    {
        ifstream dataFile(ogFile, ios::binary);
        if (!dataFile.is_open())
        {
            cerr << "Failed to open file for reading: " << ogFile << endl;
            return;
        }
        cout << "all good" << endl;
        string data((istreambuf_iterator<char>(dataFile)), istreambuf_iterator<char>());
        cout << data << endl;
        dataFile.close();

        buildCharFreqTable(data);
        buildTree();
        buildCodes(root, "");

        // Debugging: Print Huffman Table
        cout << "Huffman Table: " << endl;
        for (auto &pair : huffTable)
        {
            cout << pair.first << ": " << pair.second << endl;
        }

        writeCompressedFile(compFile, data);
    }

    void beginDecompression(string &compFile, string &decompFile)
    {
        readCompressedFile(compFile, decompFile);
        deleteTree(root);
    }
};

int main()
{
    string file1 = "sample.txt";
    string file2 = "compressedSample.txt";
    string file3 = "decompressedSample.txt";

    Huffman huff;

    cout << "Beginning File Compression" << endl;
    huff.beginCompression(file1, file2);
    cout << "File Compression Complete" << endl;
    cout << "Beginning File Decompression" << endl;
    huff.beginDecompression(file2, file3);
    cout << "File Decompression Complete" << endl;

    return 0;
}
