#pragma once

#include "Core/Core.hpp"

namespace IE::Audio {
class Sound : public IEAspect {
public:
    Sound(IE::Core::File *t_file) : m_file(t_file) {
    }

    enum AudioEncoding {
        IE_AUDIO_ENCODING_OPUS,
    };

    IE::Core::File *m_file;
    unsigned char   m_bitDepth;
    size_t          m_size;  // In samples
    AudioEncoding   m_encoding;

    template<typename T>
    bool getNextSamples(size_t t_numSamples, T *t_samples) {
        bool readAllSamples{true};
        if (m_readHead + t_numSamples >= m_size) {
            t_numSamples   = m_size - m_readHead;
            readAllSamples = false;
        }
        std::memcpy(t_samples, m_file->read(t_numSamples, m_readHead).data(), t_numSamples);
        m_readHead += t_numSamples;
        return readAllSamples;
    }

private:
    size_t m_readHead;
};
}  // namespace IE::Audio