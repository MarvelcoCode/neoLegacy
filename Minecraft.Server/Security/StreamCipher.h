#pragma once

#include <cstdint>

namespace ServerRuntime
{
	namespace Security
	{
		/**
		 * Lightweight XOR stream cipher for traffic obfuscation.
		 *
		 * This is NOT cryptographically secure. It prevents passive packet sniffing
		 * (e.g., Wireshark-based XUID harvesting) but does not protect against
		 * active man-in-the-middle attacks. For real encryption, use TLS via a
		 * reverse proxy (stunnel, nginx stream).
		 *
		 * Usage:
		 * 1. Server generates a random 16-byte key during PreLogin handshake
		 * 2. Key is sent to the client (in a SecurityHandshakePacket)
		 * 3. Both sides create a StreamCipher with the same key
		 * 4. All subsequent TCP traffic is XOR'd through the cipher
		 * 5. The cipher maintains separate send/recv rolling key positions
		 */
		class StreamCipher
		{
		public:
			static const int KEY_SIZE = 16;

			StreamCipher();

			/**
			 * Initialize with a key. Call before any encrypt/decrypt.
			 */
			void Initialize(const uint8_t key[KEY_SIZE]);

			/**
			 * XOR-encrypt data in place for sending.
			 * Advances the send key position.
			 */
			void Encrypt(uint8_t *data, int length);

			/**
			 * XOR-decrypt data in place after receiving.
			 * Advances the recv key position.
			 */
			void Decrypt(uint8_t *data, int length);

			/**
			 * Returns true if the cipher has been initialized with a key.
			 */
			bool IsActive() const { return m_active; }

			/**
			 * Reset to inactive state and securely wipe key material.
			 */
			void Reset();

			/**
			 * Generates a cryptographically random key using CryptGenRandom (Windows).
			 */
			static bool GenerateKey(uint8_t outKey[KEY_SIZE]);

		private:
			uint8_t m_key[KEY_SIZE];
			int m_sendPos;
			int m_recvPos;
			bool m_active;
		};
	}
}
