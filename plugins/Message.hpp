//
// Created by fgrotsch on 3/5/21.
//

#ifndef DAQMODULETEST_MESSAGE_HPP
#define DAQMODULETEST_MESSAGE_HPP

#define MESSAGE_SIZE 4096
#define NUMBER_OF_BUFFER_ELEMENTS (MESSAGE_SIZE / sizeof(uint_fast32_t))

namespace dunedaq {
    namespace daqmoduletest {
        struct Message {
            uint_fast32_t buffer[NUMBER_OF_BUFFER_ELEMENTS];
        };

        // For debugging purposes
        std::ostream& operator<<(std::ostream& os, const Message& message) {
            for (uint i = 0; i < NUMBER_OF_BUFFER_ELEMENTS; ++i) {
                os << message.buffer[i] << " ";
            }
            return os;
        }
    }
}

#endif //DAQMODULETEST_MESSAGE_HPP