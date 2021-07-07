// Copyright (c) 2020 Tom Hancocks
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#if !defined(KDL_RESOURCE_TRACKING_HPP)
#define KDL_RESOURCE_TRACKING_HPP

#include <string>
#include <vector>
#include <map>

namespace kdl::resource_tracking {

    class table
    {
    private:

        struct instance
        {
            std::string file;
            std::string type_code;
            std::map<std::string, std::string> attributes {};
            int64_t id;
            std::string name;

            instance(std::string file, std::string type, int64_t id, std::string name);
        };

        std::vector<instance> m_instances {};

    public:
        table() = default;

        auto add_instance(const std::string& file, const std::string& type, const int64_t& id, const std::string& name) -> void;
        [[nodiscard]] auto instance_exists(const std::string& type, const int64_t& id) -> bool;

        [[nodiscard]] auto next_available_id(const std::string& type) const -> int64_t;

    };

}

#endif //KDL_RESOURCE_TRACKING_HPP
