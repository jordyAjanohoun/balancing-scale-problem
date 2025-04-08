#include <algorithm>
#include <array>
#include <cctype>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
#include <memory>
#include <sstream>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

namespace fs = std::filesystem;

using ScaleName = std::string;
using Mass = unsigned long long;
using MassOrScaleName = std::variant<Mass, ScaleName>;

struct ScaleDescription
{
    MassOrScaleName m_left;
    MassOrScaleName m_right;
};

using ScaleMap = std::unordered_map<ScaleName, ScaleDescription>;

std::pair<ScaleMap, ScaleName> parse_input_file(const fs::path &file_path)
{
    std::ifstream file(file_path);

    if (!file)
    {
        throw std::runtime_error("Failed to open file: " + file_path.string());
    }

    ScaleMap scale_map;
    std::unordered_map<ScaleName, size_t> scale_link_tracker;

    std::string line;
    while (std::getline(file, line))
    {
        if (line.empty() || line[0] == '#')
        {
            // Skip empty lines and comments.
            continue;
        }

        // We consider the input invalid if the scale name has whitespaces.
        std::replace(line.begin(), line.end(), ',', ' ');
        std::istringstream iss(line);

        ScaleName scale_name;
        if (!(iss >> scale_name))
        {
            throw std::runtime_error("Failed to read scale name from line: " + line);
        }

        scale_link_tracker[scale_name]++;

        std::array<std::string, 2> left_right_str;
        if (!(iss >> left_right_str[0] >> left_right_str[1]))
        {
            throw std::runtime_error("Failed to read left and/or right for scale from line: " + line);
        }

        std::array<MassOrScaleName, 2> left_right;
        for (int i = 0; i < 2; i++)
        {
            if (std::isdigit(left_right_str[i][0]))
            {
                // We consider the input invalid if std::out_of_range is thrown.
                left_right[i] = std::stoull(left_right_str[i]);
            }
            else if (std::isalpha(left_right_str[i][0]))
            {
                left_right[i] = left_right_str[i];
                scale_link_tracker[left_right_str[i]]--;
            }
            else
            {
                throw std::runtime_error("Invalid mass or scale name in line: " + line);
            }
        }

        const bool success = scale_map.emplace(scale_name, ScaleDescription{left_right[0], left_right[1]}).second;

        if (!success)
        {
            throw std::runtime_error("The input is invalid. Duplicate scale name found: " + scale_name);
        }
    }

    if (scale_map.empty())
    {
        throw std::runtime_error("Invalid input file because there is no scale tree described in it: " + file_path.string());
    }

    std::vector<std::pair<ScaleName, size_t>> ill_formed_scales;

    std::copy_if(scale_link_tracker.begin(), scale_link_tracker.end(),
                 std::back_inserter(ill_formed_scales),
                 [](const auto &pair)
                 { return pair.second != 0; });

    if (ill_formed_scales.size() != 1)
    {
        std::ostringstream oss;
        oss << "Invalid input file because there are multiple ill-formed scales: ";
        for (const auto &pair : ill_formed_scales)
        {
            oss << pair.first << " ";
        }
        throw std::runtime_error(oss.str());
    }

    if (ill_formed_scales.front().second != 1)
    {
        throw std::runtime_error("Invalid input file because the ill-formed scale is not a root scale: " + ill_formed_scales.front().first);
    }

    return {scale_map, ill_formed_scales[0].first};
}

struct BalanceMasses
{
    Mass m_left;
    Mass m_right;
};

Mass balance_scale(const ScaleMap &scale_map, const std::string &scale_name, std::map<ScaleName, BalanceMasses> &balance_masses)
{
    const auto it = scale_map.find(scale_name);
    if (it == scale_map.end())
    {
        throw std::runtime_error("Failed to find scale with name " + scale_name + " in the scale map.");
    }

    const auto get_side_mass = [&scale_map, &balance_masses](const MassOrScaleName &mass_or_scale)
    {
        if (const auto mass = std::get_if<Mass>(&mass_or_scale))
        {
            return *mass;
        }
        else if (const auto scale_name = std::get_if<ScaleName>(&mass_or_scale))
        {
            return balance_scale(scale_map, *scale_name, balance_masses);
        }
        else
        {
            throw std::runtime_error("Unexpected type in scale description.");
        }
    };

    const Mass left_mass = get_side_mass(it->second.m_left);
    const Mass right_mass = get_side_mass(it->second.m_right);

    const Mass additional_right_mass = (left_mass > right_mass) ? left_mass - right_mass : 0;
    const Mass additional_left_mass = (left_mass < right_mass) ? right_mass - left_mass : 0;

    const auto success = balance_masses.emplace(scale_name, BalanceMasses{additional_left_mass, additional_right_mass}).second;

    if (!success)
    {
        throw std::runtime_error("Unexpected duplicate scale name: " + scale_name);
    }

    return std::max(left_mass, right_mass) * 2 + 1;
}

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        std::cerr << "Invalid args. Usage: " << argv[0] << " <input_file_path>" << std::endl;
        return 1;
    }

    auto [scale_map, root_scale_name] = parse_input_file(argv[1]);

    std::map<ScaleName, BalanceMasses> balance_masses;

    balance_scale(scale_map, root_scale_name, balance_masses);

    for (const auto &[scale_name, balance_mass] : balance_masses)
    {
        std::cout << scale_name << "," << balance_mass.m_left << "," << balance_mass.m_right << std::endl;
    }

    return 0;
}