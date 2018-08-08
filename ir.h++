/*
    codegen – a dynamic code generation library

    Copyright 2018 Oskari Teirilä

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/

#include "common.h++"

#ifndef CODEGEN_IR_H
#define CODEGEN_IR_H

namespace codegen
{
    namespace ir
    {
        // If there ever is a platform where sizeof(intptr_t) > sizeof(int_least64_t), this needs
        // to be defined as intptr_t on it.
        using word = std::int_least64_t;

        class buffer
        {
            std::vector<byte> _data;

        public:

            template<bool REVERSE> class iterator
            {
                const buffer *_buf;
                word _pos;

                iterator &next()
                {
                    if (_pos >= 0) _buf->read(_pos);
                    else _pos = 0;
                    return *this;
                }

                iterator &prev()
                {
                    if (_pos > 0) _pos = _buf->prev(_pos);
                    else _pos = -1;
                    return *this;
                }

            public:

                iterator(const buffer *buf, word pos) : _buf(buf), _pos(pos) { }

                word operator*() const
                {
                    return _buf->read_at(_pos);
                }

                bool operator==(const iterator &other) const
                {
                    return other._buf == _buf && other._pos == _pos;
                }

                iterator &operator++()
                {
                    return REVERSE? prev() : next();
                }

                iterator &operator--()
                {
                    return REVERSE? next() : prev();
                }

                iterator operator++(int)
                {
                    auto i = iterator(_buf, _pos);
                    ++*this;
                    return i;
                }

                iterator operator--(int)
                {
                    auto i = iterator(_buf, _pos);
                    --*this;
                    return i;
                }
            };

            void clear()
            {
                _data.clear();
            }

            std::size_t size() const
            {
                return _data.size();
            }

            void write(word x)
            {
                std::uint_least64_t w = x < 0? (-x << 1) | 1 : x << 1;
                do
                {
                    byte b = w & 0x7f;
                    w >>= 7;
                    if (w) b |= 0x80;
                    _data.push_back(b);
                }
                while (w);
            }

            iterator<false> begin() const
            {
                return iterator<false>(this, 0);
            }

            iterator<false> end() const
            {
                return iterator<false>(this, size());
            }

            iterator<true> rbegin() const
            {
                return ++iterator<true>(this, size());
            }

            iterator<true> rend() const
            {
                return iterator<true>(this, -1);
            }

            word read(word &pos) const
            {
                std::uint_least64_t w = _data[pos] & 0x7f;
                for (int i = 1; _data[pos++] & 0x80; ++i) w |= (std::uint_least64_t)(_data[pos] & 0x7f) << i * 7;
                return w & 1? -(w >> 1) : w >> 1;
            }

            word read_at(word pos) const
            {
                word p = pos;
                return read(p);
            }

            word prev(word pos) const
            {
                if (pos < 0) return -1;
                while (--pos > 0 && (_data[pos - 1] & 0x80));
                return pos;
            }
        };
    };
}

#endif
