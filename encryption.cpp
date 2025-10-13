#include "include/common/encryption.h"

uint32_t EncryptionHelper::resource_decrypt(uint32_t encrypted_value) {
    return encrypted_value ^ RESOURCE_DATA_XOR_KEY;
}

uint32_t EncryptionHelper::resource_encrypt(uint32_t plain_value) {
    return plain_value ^ RESOURCE_DATA_XOR_KEY;
}

uint32_t EncryptionHelper::player_building_decrypt(uint32_t encrypted_value) {
    return encrypted_value ^ PLAYER_BUILDING_DATA_XOR_KEY;
}

uint32_t EncryptionHelper::player_building_encrypt(uint32_t plain_value) {
    return plain_value ^ PLAYER_BUILDING_DATA_XOR_KEY;
}
