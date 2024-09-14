#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>

typedef struct _LINKED_LIST {
    unsigned char* pBuffer;
    int ID;
    struct _LINKED_LIST* Next;
} LINKED_LIST, *PLINKED_LIST;

// Huffman Tree node
typedef struct _HUFFMAN_NODE {
    unsigned char data;
    unsigned freq;
    struct _HUFFMAN_NODE* left, * right;
} HUFFMAN_NODE;

// Structure for Min Heap
typedef struct _MIN_HEAP {
    unsigned size;
    unsigned capacity;
    HUFFMAN_NODE** array;
} MIN_HEAP;

typedef enum {
    COMPRESSION_HUFFMAN,
    COMPRESSION_RLE_HUFFMAN
} COMPRESSION_METHOD;


PLINKED_LIST InsertAtTheEnd(PLINKED_LIST LinkedList, unsigned char* pBuffer, int ID);

// Utility functions for Huffman encoding
HUFFMAN_NODE* newNode(unsigned char data, unsigned freq) {
    HUFFMAN_NODE* temp = (HUFFMAN_NODE*)calloc(1, sizeof(HUFFMAN_NODE));
    temp->data = data;
    temp->freq = freq;
    temp->left = temp->right = NULL;
    return temp;
}

MIN_HEAP* createMinHeap(unsigned capacity) {
    MIN_HEAP* minHeap = (MIN_HEAP*)calloc(1, sizeof(MIN_HEAP));
    minHeap->size = 0;
    minHeap->capacity = capacity;
    minHeap->array = (HUFFMAN_NODE**)calloc(minHeap->capacity, sizeof(HUFFMAN_NODE*));
    return minHeap;
}

void swapNodes(HUFFMAN_NODE** a, HUFFMAN_NODE** b) {
    HUFFMAN_NODE* t = *a;
    *a = *b;
    *b = t;
}

void minHeapify(MIN_HEAP* minHeap, int idx) {
    int smallest = idx;
    int left = 2 * idx + 1;
    int right = 2 * idx + 2;

    if (left < minHeap->size && minHeap->array[left]->freq < minHeap->array[smallest]->freq)
        smallest = left;

    if (right < minHeap->size && minHeap->array[right]->freq < minHeap->array[smallest]->freq)
        smallest = right;

    if (smallest != idx) {
        swapNodes(&minHeap->array[smallest], &minHeap->array[idx]);
        minHeapify(minHeap, smallest);
    }
}

int isSizeOne(MIN_HEAP* minHeap) {
    return (minHeap->size == 1);
}

HUFFMAN_NODE* extractMin(MIN_HEAP* minHeap) {
    HUFFMAN_NODE* temp = minHeap->array[0];
    minHeap->array[0] = minHeap->array[minHeap->size - 1];
    --minHeap->size;
    minHeapify(minHeap, 0);
    return temp;
}

void insertMinHeap(MIN_HEAP* minHeap, HUFFMAN_NODE* huffmanNode) {
    ++minHeap->size;
    int i = minHeap->size - 1;

    while (i && huffmanNode->freq < minHeap->array[(i - 1) / 2]->freq) {
        minHeap->array[i] = minHeap->array[(i - 1) / 2];
        i = (i - 1) / 2;
    }
    minHeap->array[i] = huffmanNode;
}

void buildMinHeap(MIN_HEAP* minHeap) {
    int n = minHeap->size - 1;
    for (int i = (n - 1) / 2; i >= 0; --i)
        minHeapify(minHeap, i);
}

int isLeaf(HUFFMAN_NODE* root) {
    return !(root->left) && !(root->right);
}

MIN_HEAP* createAndBuildMinHeap(unsigned char data[], int freq[], int size) {
    MIN_HEAP* minHeap = createMinHeap(size);
    for (int i = 0; i < size; ++i)
        minHeap->array[i] = newNode(data[i], freq[i]);
    minHeap->size = size;
    buildMinHeap(minHeap);
    return minHeap;
}

HUFFMAN_NODE* buildHuffmanTree(unsigned char data[], int freq[], int size) {
    HUFFMAN_NODE *left, *right, *top;
    MIN_HEAP* minHeap = createAndBuildMinHeap(data, freq, size);

    while (!isSizeOne(minHeap)) {
        left = extractMin(minHeap);
        right = extractMin(minHeap);

        top = newNode('$', left->freq + right->freq);
        top->left = left;
        top->right = right;

        insertMinHeap(minHeap, top);
    }
    return extractMin(minHeap);
}

void printCodes(HUFFMAN_NODE* root, int arr[], int top) {
    if (root->left) {
        arr[top] = 0;
        printCodes(root->left, arr, top + 1);
    }

    if (root->right) {
        arr[top] = 1;
        printCodes(root->right, arr, top + 1);
    }

    if (isLeaf(root)) {
        printf("%c: ", root->data);
        for (int i = 0; i < top; ++i)
            printf("%d", arr[i]);
        printf("\n");
    }
}

void HuffmanCodes(unsigned char data[], int freq[], int size) {
    HUFFMAN_NODE* root = buildHuffmanTree(data, freq, size);
    int arr[100], top = 0;
    printCodes(root, arr, top);
}

// Utility function to find the nearest multiple of a given number
size_t NEAREST_MULTIPLE(size_t num, size_t multiple) {
    return ((num + multiple - 1) / multiple) * multiple;
}

// Function to insert a node at the end of the linked list
PLINKED_LIST InsertAtTheEnd(PLINKED_LIST LinkedList, unsigned char* pBuffer, int ID) {
    PLINKED_LIST pTmpHead = LinkedList;

    PLINKED_LIST pNewNode = (PLINKED_LIST)calloc(1, sizeof(LINKED_LIST));
    if (!pNewNode)
        return NULL;

    pNewNode->pBuffer = (unsigned char*)calloc(16, sizeof(unsigned char)); // Assuming BUFF_SIZE = 16
    memcpy(pNewNode->pBuffer, pBuffer, 16);
    pNewNode->ID = ID;
    pNewNode->Next = NULL;

    if (LinkedList == NULL) {
        LinkedList = pNewNode;
        return LinkedList;
    }

    while (pTmpHead->Next != NULL)
        pTmpHead = pTmpHead->Next;

    pTmpHead->Next = pNewNode;
    return LinkedList;
}


// Function to write the encoded linked list data to a file
void WriteLinkedListToFile(PLINKED_LIST linkedList, const char* fileName) {
    FILE *file = fopen(fileName, "wb");
    if (file == NULL) {
        perror("Failed to open file for writing");
        exit(EXIT_FAILURE);
    }

    PLINKED_LIST current = linkedList;
    while (current != NULL) {
        fwrite(&current->ID, sizeof(current->ID), 1, file);
        fwrite(current->pBuffer, 16, 1, file); // Assuming BUFF_SIZE = 16
        current = current->Next;
    }

    fclose(file);
}

// Function to read encoded data from a file into a linked list
PLINKED_LIST ReadLinkedListFromFile(const char* fileName) {
    FILE *file = fopen(fileName, "rb");
    if (file == NULL) {
        perror("Failed to open file for reading");
        exit(EXIT_FAILURE);
    }

    PLINKED_LIST linkedList = NULL;
    int id;
    unsigned char buffer[16];

    while (fread(&id, sizeof(id), 1, file) == 1) {
        if (fread(buffer, 16, 1, file) != 1) {
            perror("Error reading buffer from file");
            fclose(file);
            exit(EXIT_FAILURE);
        }
        linkedList = InsertAtTheEnd(linkedList, buffer, id);
    }

    fclose(file);
    return linkedList;
}

// Function to decode the linked list back to the original payload
unsigned char* DecodeLinkedList(PLINKED_LIST linkedList, size_t* payloadSize) {
    *payloadSize = 0;
    PLINKED_LIST current = linkedList;

    while (current != NULL) {
        *payloadSize += 16; // Assuming BUFF_SIZE = 16
        current = current->Next;
    }

    unsigned char* decodedPayload = (unsigned char*)malloc(*payloadSize);
    if (decodedPayload == NULL) {
        perror("Failed to allocate memory for decoded payload");
        exit(EXIT_FAILURE);
    }

    current = linkedList;
    size_t offset = 0;

    while (current != NULL) {
        memcpy(decodedPayload + offset, current->pBuffer, 16);
        offset += 16;
        current = current->Next;
    }

    return decodedPayload;
}

// RLEEncode function definition
unsigned char* RLEEncode(unsigned char* data, size_t dataSize, size_t* encodedSize) {
    unsigned char* encodedData = (unsigned char*)malloc(2 * dataSize); // Worst case, encoded data is double the size
    if (!encodedData) {
        perror("Failed to allocate memory for RLE encoded data");
        exit(EXIT_FAILURE);
    }

    size_t writeIndex = 0;
    for (size_t i = 0; i < dataSize; ) {
        unsigned char currentByte = data[i];
        size_t runLength = 1;

        while (i + runLength < dataSize && data[i + runLength] == currentByte && runLength < 255) {
            runLength++;
        }

        encodedData[writeIndex++] = currentByte;
        encodedData[writeIndex++] = (unsigned char)runLength;

        i += runLength;
    }

    *encodedSize = writeIndex;
    return encodedData;
}


int InitializePayloadList(unsigned char* pPayload, size_t* sPayloadSize, PLINKED_LIST* ppLinkedList, COMPRESSION_METHOD method) {
    unsigned int x = 0;
    unsigned char* dataToCompress = pPayload;
    size_t dataSize = *sPayloadSize;

    if (method == COMPRESSION_RLE_HUFFMAN) {
        // Apply Run-Length Encoding first
        size_t rleEncodedSize;
        dataToCompress = RLEEncode(pPayload, *sPayloadSize, &rleEncodedSize);
        dataSize = rleEncodedSize;
    }

    // Apply Huffman encoding to the selected data (either original or RLE encoded)
    int freq[256] = { 0 };
    for (size_t i = 0; i < dataSize; i++) {
        freq[dataToCompress[i]]++;
    }

    unsigned char data[256];
    int dataCount = 0;
    for (int i = 0; i < 256; i++) {
        if (freq[i] > 0) {
            data[dataCount++] = (unsigned char)i;
        }
    }

    HuffmanCodes(data, freq, dataCount);

    // setting the payload size to be multiple of 'BUFF_SIZE'
    const size_t BUFF_SIZE = 16;
    const size_t SERIALIZED_SIZE = sizeof(LINKED_LIST);

    size_t sTmpSize = NEAREST_MULTIPLE(dataSize, BUFF_SIZE);
    if (!sTmpSize) {
        if (method == COMPRESSION_RLE_HUFFMAN) {
            free(dataToCompress);  // Free the RLE encoded data if it was allocated
        }
        return 0;
    }

    // new padded buffer
    unsigned char* pTmpBuffer = (unsigned char*)calloc(sTmpSize, sizeof(unsigned char));
    if (!pTmpBuffer) {
        if (method == COMPRESSION_RLE_HUFFMAN) {
            free(dataToCompress);
        }
        return 0;
    }

    memcpy(pTmpBuffer, dataToCompress, dataSize);

    if (method == COMPRESSION_RLE_HUFFMAN) {
        free(dataToCompress);  // Free the RLE encoded data if it was allocated
    }

    // for each 'BUFF_SIZE' in the padded payload, add it to the linked list
    for (size_t i = 0; i < sTmpSize; i++) {
        if (i % BUFF_SIZE == 0) {
            *ppLinkedList = InsertAtTheEnd((PLINKED_LIST)*ppLinkedList, &pTmpBuffer[i], x);
            x++;
        }
    }

    // updating the size to be the size of the whole *serialized* linked list
    *sPayloadSize = SERIALIZED_SIZE * x;

    if (*ppLinkedList == NULL) {
        free(pTmpBuffer);
        return 0;
    }

    free(pTmpBuffer); // Free temporary buffer
    return 1;
}

int main(int argc, char* argv[]) {
    if (argc < 5) {
        fprintf(stderr, "Usage: %s <encode/decode> <huffman/rle-huffman> <input_file> <output_file>\n", argv[0]);
        return EXIT_FAILURE;
    }

    const char* mode = argv[1];
    const char* compressionMethodStr = argv[2];
    const char* inputFileName = argv[3];
    const char* outputFileName = argv[4];

    COMPRESSION_METHOD method;

    if (strcmp(compressionMethodStr, "huffman") == 0) {
        method = COMPRESSION_HUFFMAN;
    } else if (strcmp(compressionMethodStr, "rle-huffman") == 0) {
        method = COMPRESSION_RLE_HUFFMAN;
    } else {
        fprintf(stderr, "Invalid compression method. Use 'huffman' or 'rle-huffman'.\n");
        return EXIT_FAILURE;
    }

    if (strcmp(mode, "encode") == 0) {
        // Read the input file into a buffer
        FILE *inputFile = fopen(inputFileName, "rb");
        if (inputFile == NULL) {
            perror("Failed to open input file");
            return EXIT_FAILURE;
        }

        fseek(inputFile, 0, SEEK_END);
        size_t fileSize = ftell(inputFile);
        fseek(inputFile, 0, SEEK_SET);

        unsigned char* fileBuffer = (unsigned char*)malloc(fileSize);
        if (fileBuffer == NULL) {
            perror("Failed to allocate memory for file buffer");
            fclose(inputFile);
            return EXIT_FAILURE;
        }

        fread(fileBuffer, 1, fileSize, inputFile);
        fclose(inputFile);

        // Encode the buffer and store it in a linked list
        PLINKED_LIST linkedList = NULL;
        InitializePayloadList(fileBuffer, &fileSize, &linkedList, method);

        // Write the encoded linked list to the output file
        WriteLinkedListToFile(linkedList, outputFileName);

        free(fileBuffer);
        printf("File successfully encoded using %s method and written to %s.\n", compressionMethodStr, outputFileName);

    } else if (strcmp(mode, "decode") == 0) {
        // Read the encoded linked list from the input file
        PLINKED_LIST linkedList = ReadLinkedListFromFile(inputFileName);

        // Decode the linked list back to the original payload
        size_t decodedSize;
        unsigned char* decodedPayload = DecodeLinkedList(linkedList, &decodedSize);

        // Write the decoded payload to the output file
        FILE *outputFile = fopen(outputFileName, "wb");
        if (outputFile == NULL) {
            perror("Failed to open output file");
            free(decodedPayload);
            return EXIT_FAILURE;
        }

        fwrite(decodedPayload, 1, decodedSize, outputFile);
        fclose(outputFile);

        free(decodedPayload);
        printf("File successfully decoded and written to %s.\n", outputFileName);

    } else {
        fprintf(stderr, "Invalid mode. Use 'encode' or 'decode'.\n");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

