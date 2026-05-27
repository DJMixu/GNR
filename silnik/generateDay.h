//
// Created by foxyu on 16.04.2026.
//

#ifndef SILNIK_GENERATEDAY_H
#define SILNIK_GENERATEDAY_H
#pragma once
#include <string>

void generate_day(const std::string& source_path, const std::string& output_path);
void ensure_days_exist(const std::string& source_file, int number_of_days, bool overwrite);


#endif //SILNIK_GENERATEDAY_H
