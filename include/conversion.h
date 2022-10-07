//
// Created by Hansol Lee on 2022-10-02.
//

#ifndef FILE_TRANSFER_HL_CONVERSION_H
#define FILE_TRANSFER_HL_CONVERSION_H

#include <netinet/in.h>

in_port_t parse_port(const char *buff, int radix);

#endif //FILE_TRANSFER_HL_CONVERSION_H
