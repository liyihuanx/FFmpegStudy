//
// Created by leeyh on 2022/10/13.
//

#ifndef FFMPEGSTUDY_AUDIOFRAME_H
#define FFMPEGSTUDY_AUDIOFRAME_H

#endif //FFMPEGSTUDY_AUDIOFRAME_H

class AudioFrame {
public:
    AudioFrame(uint8_t * data, int dataSize, bool hardCopy = true) {
        this->dataSize = dataSize;
        this->data = data;
        this->hardCopy = hardCopy;
        if(hardCopy) {
            this->data = static_cast<uint8_t *>(malloc(this->dataSize));
            memcpy(this->data, data, dataSize);
        }
    }

    ~AudioFrame() {
        if(hardCopy && this->data)
            free(this->data);
        this->data = nullptr;
    }

    uint8_t * data = nullptr;
    int dataSize = 0;
    bool hardCopy = true;
};