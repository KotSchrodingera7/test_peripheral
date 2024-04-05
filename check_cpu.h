#pragma once

#include <unistd.h>
#include <iostream>
#include <fstream>
#include <optional>
#include <vector>
#include <numeric>
#include <cmath>
#include <array>

std::istream& operator>>(std::istream &in, std::vector<uint32_t> &time);

class CheckCpu 
{
public:
    CheckCpu() = default;

    std::vector<double> GetFreq()
    {
        std::vector<double> result;
        auto value = ReadTime();

        if( value.size() > 0 )
        {
            int i = 0;
            for(const auto [user_now, total_now] : value)
            {
                unsigned int user_over_period = user_now - (time_[i].first); 
                unsigned int total_over_period = total_now - time_[i].second;
                double cpu_usage = static_cast<double>(user_over_period)/total_over_period * 100;

                time_[i].second = total_now;
                time_[i].first = user_now;
                cpu_usage = std::round(cpu_usage*100)/100;
                result.push_back(cpu_usage);
                ++i;
            }
        }

        return result;
    }
private:
    std::vector<std::pair<uint32_t, uint32_t>> ReadTime()
    {
        std::vector<std::pair<uint32_t, uint32_t>> result;
        std::ifstream file("/proc/stat");
        
        uint32_t user_now = 0, total_now = 0;
        if( file.is_open() )
        {
            std::vector<uint32_t> timing(10, 0);
            file >> timing;
            for(int i = 0; i < 4; ++i)
            {
                file >> timing;
                user_now = std::accumulate(timing.begin(), timing.begin() + 3, 0);
                total_now = std::accumulate(timing.begin(), timing.end(), 0);

                result.push_back({user_now, total_now});
            }
        } else 
        {
            std::cout << "File is not open" << std::endl;
        }

        return result;
    }

private:
    std::array<std::pair<uint32_t, uint32_t>, 4> time_;
};