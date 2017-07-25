/* Keyleds -- Gaming keyboard tool
 * Copyright (C) 2017 Julien Hartmann, juli1.hartmann@gmail.com
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef TOOLS_PATH_H_B703CD61
#define TOOLS_PATH_H_B703CD61

#include <iosfwd>
#include <string>
#include <vector>

namespace paths {

enum class XDG { Cache, Config, Data, Runtime };

template <typename T> struct stream_attributes{};
template <> struct stream_attributes<std::ifstream> {
    static constexpr std::ios::openmode default_mode = std::ios::in;
};
template <> struct stream_attributes<std::ofstream> {
    static constexpr std::ios::openmode default_mode = std::ios::out;
};

std::filebuf open_filebuf(XDG type, const std::string & path, std::ios_base::openmode);
std::vector<std::string> getPaths(XDG, bool extra);

template <typename T> T open(XDG type, const std::string & path, typename T::openmode mode)
{
    auto result = T{};
    mode |= stream_attributes<T>::default_mode;
    *result.rdbuf() = open_filebuf(type, path, mode);
    return result;
}

}

#endif