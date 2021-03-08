//
// Created by fgrotsch on 3/5/21.
//

#ifndef DAQMODULETEST_MESSAGE_HPP
#define DAQMODULETEST_MESSAGE_HPP

#include "message_size_conf.h"

#define NUMBER_OF_BUFFER_ELEMENTS (MESSAGE_SIZE / sizeof(uint32_t))

namespace dunedaq {
    namespace daqmoduletest {
        struct Message {
            uint32_t buffer[NUMBER_OF_BUFFER_ELEMENTS];
        };

        std::ostream& operator<<(std::ostream& os, const Message& message) {
            for (uint i = 0; i < NUMBER_OF_BUFFER_ELEMENTS; ++i) {
                os << message.buffer[i] << " ";
            }
            return os;
        }
    }
}

#endif //DAQMODULETEST_MESSAGE_HPP
