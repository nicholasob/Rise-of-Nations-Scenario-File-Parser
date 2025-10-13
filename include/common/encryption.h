#pragma once
#include <cstdint>

class EncryptionHelper {
public:

    static constexpr uint32_t RESOURCE_DATA_XOR_KEY = 0x8221;
    static constexpr uint32_t PLAYER_BUILDING_DATA_XOR_KEY = 0x63637;
    
    static uint32_t resource_decrypt(uint32_t encrypted_value);
    static uint32_t resource_encrypt(uint32_t plain_value);
    
    static uint32_t player_building_decrypt(uint32_t encrypted_value);
    static uint32_t player_building_encrypt(uint32_t plain_value);

    /**
     * Little-endian!
     * 
     * Decrypts a 32-bit input by:
     * 1. Extracting byte3  = (v >> 16) & 0xFF
     * 2. Extracting byte4  = (v >> 24) & 0xFF
     * 3. Forming N         = byte3 + (byte4 << 8)
     * 4. Dividing N by 182 and discarding remainder
     */
    static uint32_t decode182Scaled(uint32_t v) {
        uint32_t byte3 = (v >> 16) & 0xFF;
        uint32_t byte4 = (v >> 24) & 0xFF;
        uint32_t N     = byte3 + (byte4 << 8);
        return N / 182;
    }

    /**
     * Little-endian!
     * 
     * Encrypts a 32-bit input by:
     * 1. Computing N         = value × 182
     * 2. Extracting byte3    =  N & 0xFF         (low 8 bits of N)
     * 3. Extracting byte4    = (N >> 8) & 0xFF   (next 8 bits of N)
     * 4. Placing them into v:
     *      • byte3 goes into bits 16–23
     *      • byte4 goes into bits 24–31
     *    result = (byte3 << 16) | (byte4 << 24)
     */
    static uint32_t encode182Scaled(uint32_t value) {
        uint32_t N     = value * 182;
        uint32_t byte3 =  N        & 0xFF;
        uint32_t byte4 = (N >> 8)  & 0xFF;
        return (byte3 << 16) | (byte4 << 24);
    }

    /**
     * XOR decryption with different key
     */
    static uint32_t xor_decrypt(uint32_t encrypted_value, uint32_t key) {
        return encrypted_value ^ key;
    }

    /**
     * XOR encryption with different key
     */
    static uint32_t xor_encrypt(uint32_t plain_value, uint32_t key) {
        return plain_value ^ key;
    }

    // Add getters for static fields
    static uint32_t get_resource_xor_key() { return RESOURCE_DATA_XOR_KEY; }
    static uint32_t get_player_building_xor_key() { return PLAYER_BUILDING_DATA_XOR_KEY; }
};
