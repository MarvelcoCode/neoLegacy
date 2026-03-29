#include "stdafx.h"
#include "StreamCipher.h"

#ifdef _WINDOWS64
#include <Windows.h>
#include <wincrypt.h>
#pragma comment(lib, "Advapi32.lib")
#endif

#include <cstring>

namespace ServerRuntime
{
	namespace Security
	{
		StreamCipher::StreamCipher()
			: m_sendPos(0)
			, m_recvPos(0)
			, m_active(false)
		{
			memset(m_key, 0, sizeof(m_key));
		}

		void StreamCipher::Initialize(const uint8_t key[KEY_SIZE])
		{
			memcpy(m_key, key, KEY_SIZE);
			m_sendPos = 0;
			m_recvPos = 0;
			m_active = true;
		}

		void StreamCipher::Reset()
		{
			SecureZeroMemory(m_key, sizeof(m_key));
			m_sendPos = 0;
			m_recvPos = 0;
			m_active = false;
		}

		void StreamCipher::Encrypt(uint8_t *data, int length)
		{
			if (!m_active || data == nullptr || length <= 0)
			{
				return;
			}

			for (int i = 0; i < length; ++i)
			{
				data[i] ^= m_key[m_sendPos];
				m_sendPos = (m_sendPos + 1) % KEY_SIZE;
			}
		}

		void StreamCipher::Decrypt(uint8_t *data, int length)
		{
			if (!m_active || data == nullptr || length <= 0)
			{
				return;
			}

			for (int i = 0; i < length; ++i)
			{
				data[i] ^= m_key[m_recvPos];
				m_recvPos = (m_recvPos + 1) % KEY_SIZE;
			}
		}

		bool StreamCipher::GenerateKey(uint8_t outKey[KEY_SIZE])
		{
#ifdef _WINDOWS64
			HCRYPTPROV hProv = 0;
			if (!CryptAcquireContext(&hProv, nullptr, nullptr, PROV_RSA_AES, CRYPT_VERIFYCONTEXT))
			{
				return false;
			}

			BOOL result = CryptGenRandom(hProv, KEY_SIZE, outKey);
			CryptReleaseContext(hProv, 0);
			return result != FALSE;
#else
			// Fallback: not cryptographically random, but better than nothing
			for (int i = 0; i < KEY_SIZE; ++i)
			{
				outKey[i] = static_cast<uint8_t>(rand() & 0xFF);
			}
			return true;
#endif
		}
	}
}
