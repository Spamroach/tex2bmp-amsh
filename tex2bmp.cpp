#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cstdint>
#include <filesystem> // Requires C++17 (Visual Studio 2017+)

// --- BMP HEADER STRUCT ---
#pragma pack(push, 1)
struct BmpHeader {
    uint16_t signature{ 0x4D42 }; // 'BM'
    uint32_t fileSize;
    uint32_t reserved{ 0 };
    uint32_t dataOffset{ 54 }; 
    uint32_t headerSize{ 40 };
    int32_t  width;
    int32_t  height;
    uint16_t planes{ 1 };
    uint16_t bpp{ 24 }; // 24-bit RGB
    uint32_t compression{ 0 };
    uint32_t imageSize{ 0 };
    int32_t  xRes{ 0 };
    int32_t  yRes{ 0 };
    uint32_t colorsUsed{ 0 };
    uint32_t colorsImportant{ 0 };
};
#pragma pack(pop)

struct Color { uint8_t r, g, b; };

// --- 1. PALETTE DECODER (Based on your successful script) ---
Color decodeRGB555(uint16_t pixel) {
    // 0 RRRRR GGGGG BBBBB
    uint8_t r = (pixel >> 10) & 0x1F;
    uint8_t g = (pixel >> 5)  & 0x1F;
    uint8_t b = pixel & 0x1F;

    // Scale 5-bit to 8-bit
    return {
        (uint8_t)((r * 255) / 31),
        (uint8_t)((g * 255) / 31),
        (uint8_t)((b * 255) / 31)
    };
}

// --- 2. MAIN CONVERSION LOGIC ---
void convertFile(const std::string& inputPath) {
    std::cout << "Processing: " << inputPath << "...\n";

    std::ifstream file(inputPath, std::ios::binary);
    if (!file) {
        std::cerr << "Error: Could not open file.\n";
        return;
    }

    // A. READ DIMENSIONS (Offsets 0x08 and 0x0C)
    int32_t width = 0;
    int32_t height = 0;

    file.seekg(0x08, std::ios::beg);
    file.read(reinterpret_cast<char*>(&width), 4);
    file.read(reinterpret_cast<char*>(&height), 4);

    std::cout << " - Dimensions: " << width << "x" << height << "\n";

    if (width <= 0 || height <= 0 || width > 4096 || height > 4096) {
        std::cerr << "Error: Invalid dimensions found in header.\n";
        return;
    }

    // B. READ IMAGE DATA (Header is 24 bytes)
    size_t headerSize = 24;
    size_t dataSize = (width * height) / 2; // 4bpp

    file.seekg(headerSize, std::ios::beg);
    std::vector<uint8_t> rawBytes(dataSize);
    file.read(reinterpret_cast<char*>(rawBytes.data()), dataSize);

    // C. READ PALETTE (Last 32 bytes)
    std::vector<Color> palette(16);
    file.seekg(-32, std::ios::end); 

    for (int i = 0; i < 16; ++i) {
        uint8_t b1 = file.get();
        uint8_t b2 = file.get();
        uint16_t raw = (b2 << 8) | b1; // Little Endian
        palette[i] = decodeRGB555(raw);
    }

    // D. UNPACK PIXELS (Using the "Swapped Nibbles" logic that worked)
    std::vector<uint8_t> pixelIndices;
    pixelIndices.reserve(width * height);

    for (uint8_t b : rawBytes) {
        // Logic from '2_Swapped_Nibbles.ppm':
        // Low Nibble  = First Pixel (Left)
        // High Nibble = Second Pixel (Right)
        pixelIndices.push_back(b & 0x0F); 
        pixelIndices.push_back((b >> 4) & 0x0F);
    }

    // E. WRITE BMP
    // Replace extension with .bmp
    std::string outputPath = inputPath.substr(0, inputPath.find_last_of('.')) + ".bmp";
    
    std::ofstream bmp(outputPath, std::ios::binary);
    if (!bmp) { std::cerr << "Error: Could not write BMP.\n"; return; }

    // BMP Header
    BmpHeader header;
    header.width = width;
    header.height = height; // Positive = Bottom-Up
    header.fileSize = sizeof(BmpHeader) + (width * height * 3);
    
    bmp.write(reinterpret_cast<const char*>(&header), sizeof(header));

    // Write Pixels (BMP stores Bottom-to-Top)
    for (int y = height - 1; y >= 0; --y) {
        for (int x = 0; x < width; ++x) {
            uint8_t idx = pixelIndices[y * width + x];
            Color c;

            if (idx == 0) {
                // Transparency Key -> Magic Pink
                c = { 0, 0, 0 }; 
            } else {
                c = palette[idx];
            }

            // BMP uses BGR order // Switched it to RGB, it fixed some textures. -Spamroach
            bmp.put(c.r);
            bmp.put(c.g);
            bmp.put(c.b);
        }
    }

    bmp.close();
    std::cout << "Success! Saved: " << outputPath << "\n";
}

// --- 3. DRAG AND DROP ENTRY POINT ---
int main(int argc, char* argv[]) {
    std::cout << "--- TEX to BMP (Swapped Nibbles / RGB555) ---\n";

    if (argc < 2) {
        std::cout << "Usage: Drag and drop .TEX files onto this executable.\n";
    } else {
        for (int i = 1; i < argc; ++i) {
            convertFile(argv[i]);
            std::cout << "---------------------------------------------\n";
        }
    }

    std::cout << "\nPress ENTER to exit...";
    std::cin.get(); 
    return 0;
}