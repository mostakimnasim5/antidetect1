#include "CryptoUtils.hpp"
#include <openssl/md5.h>
#include <openssl/sha.h>
#include <openssl/evp.h>
#include <openssl/bio.h>
#include <openssl/buffer.h>
#include <random>
#include <sstream>
#include <iomanip>
#include <regex>
#include <algorithm>

namespace AntiDetect {

std::string CryptoUtils::md5(const std::string& input) {
    unsigned char digest[MD5_DIGEST_LENGTH];
    MD5(reinterpret_cast<const unsigned char*>(input.c_str()), input.length(), digest);
    
    std::stringstream ss;
    for (int i = 0; i < MD5_DIGEST_LENGTH; ++i) {
        ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(digest[i]);
    }
    return ss.str();
}

std::string CryptoUtils::sha256(const std::string& input) {
    unsigned char digest[SHA256_DIGEST_LENGTH];
    SHA256(reinterpret_cast<const unsigned char*>(input.c_str()), input.length(), digest);
    
    std::stringstream ss;
    for (int i = 0; i < SHA256_DIGEST_LENGTH; ++i) {
        ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(digest[i]);
    }
    return ss.str();
}

std::string CryptoUtils::sha512(const std::string& input) {
    unsigned char digest[SHA512_DIGEST_LENGTH];
    SHA512(reinterpret_cast<const unsigned char*>(input.c_str()), input.length(), digest);
    
    std::stringstream ss;
    for (int i = 0; i < SHA512_DIGEST_LENGTH; ++i) {
        ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(digest[i]);
    }
    return ss.str();
}

std::string CryptoUtils::generateUUID() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 15);
    std::uniform_int_distribution<> dis2(8, 11);
    
    std::stringstream ss;
    ss << std::hex;
    ss << std::setfill('0') << std::setw(8) << dis(gen) << dis(gen) << dis(gen) << dis(gen)
       << dis(gen) << dis(gen) << dis(gen) << dis(gen) << "-";
    ss << std::setw(4) << dis(gen) << dis(gen) << dis(gen) << dis(gen) << "-";
    ss << "4" << std::setw(3) << dis(gen) << dis(gen) << dis(gen) << "-";
    ss << std::hex << (dis2(gen)) << dis(gen) << dis(gen) << dis(gen) << "-";
    ss << std::setfill('0') << std::setw(12) << dis(gen) << dis(gen) << dis(gen) 
       << dis(gen) << dis(gen) << dis(gen);
    
    return ss.str();
}

std::string CryptoUtils::generateRandomString(int length) {
    const char chars[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, sizeof(chars) - 2);
    
    std::string result;
    for (int i = 0; i < length; ++i) {
        result += chars[dis(gen)];
    }
    return result;
}

std::string CryptoUtils::generateRandomHex(int length) {
    const char chars[] = "0123456789ABCDEF";
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, sizeof(chars) - 2);
    
    std::string result;
    for (int i = 0; i < length; ++i) {
        result += chars[dis(gen)];
    }
    return result;
}

std::string CryptoUtils::base64Encode(const std::string& input) {
    BIO* bio = BIO_new(BIO_s_mem());
    BIO* b64 = BIO_new(BIO_f_base64());
    BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);
    bio = BIO_push(b64, bio);
    
    BIO_write(bio, input.c_str(), input.length());
    BIO_flush(bio);
    
    BUF_MEM* bufferPtr;
    BIO_get_mem_ptr(bio, &bufferPtr);
    
    std::string result(bufferPtr->data, bufferPtr->length);
    BIO_free_all(bio);
    
    return result;
}

std::string CryptoUtils::base64Decode(const std::string& input) {
    BIO* bio = BIO_new_mem_buf(input.c_str(), input.length());
    BIO* b64 = BIO_new(BIO_f_base64());
    BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);
    bio = BIO_push(b64, bio);
    
    std::vector<char> buffer(input.length());
    int decodedLength = BIO_read(bio, buffer.data(), input.length());
    
    std::string result(buffer.data(), decodedLength);
    BIO_free_all(bio);
    
    return result;
}

std::string CryptoUtils::xorEncrypt(const std::string& input, const std::string& key) {
    std::string output = input;
    for (size_t i = 0; i < input.length(); ++i) {
        output[i] = input[i] ^ key[i % key.length()];
    }
    return output;
}

std::string CryptoUtils::xorDecrypt(const std::string& input, const std::string& key) {
    return xorEncrypt(input, key);
}

bool CryptoUtils::isValidHex(const std::string& hex) {
    return std::regex_match(hex, std::regex("^[0-9A-Fa-f]+$"));
}

bool CryptoUtils::isValidMAC(const std::string& mac) {
    std::regex macRegex("^([0-9A-Fa-f]{2}:){5}[0-9A-Fa-f]{2}$");
    return std::regex_match(mac, macRegex);
}

bool CryptoUtils::isValidIPv4(const std::string& ip) {
    std::regex ipv4Regex("^((25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\.){3}(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)$");
    return std::regex_match(ip, ipv4Regex);
}

bool CryptoUtils::isValidIPv6(const std::string& ip) {
    std::regex ipv6Regex("^([0-9a-fA-F]{1,4}:){7}[0-9a-fA-F]{1,4}$");
    return std::regex_match(ip, ipv6Regex) || ip == "::1" || ip == "::";
}

std::string CryptoUtils::sanitizeString(const std::string& input) {
    std::string output;
    for (char c : input) {
        if (c >= 32 && c < 127) {
            output += c;
        }
    }
    return output;
}

std::string CryptoUtils::escapeString(const std::string& input) {
    std::string output;
    for (char c : input) {
        switch (c) {
            case '\\': output += "\\\\"; break;
            case '"': output += "\\\""; break;
            case '\n': output += "\\n"; break;
            case '\r': output += "\\r"; break;
            case '\t': output += "\\t"; break;
            default: output += c; break;
        }
    }
    return output;
}

std::vector<unsigned char> CryptoUtils::hexToBytes(const std::string& hex) {
    std::vector<unsigned char> bytes;
    for (size_t i = 0; i < hex.length(); i += 2) {
        std::string byteStr = hex.substr(i, 2);
        unsigned char byte = static_cast<unsigned char>(std::stoi(byteStr, nullptr, 16));
        bytes.push_back(byte);
    }
    return bytes;
}

std::string CryptoUtils::bytesToHex(const std::vector<unsigned char>& bytes) {
    std::stringstream ss;
    ss << std::hex << std::setfill('0');
    for (unsigned char byte : bytes) {
        ss << std::setw(2) << static_cast<int>(byte);
    }
    return ss.str();
}

}
