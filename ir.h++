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

            word size() const
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

        struct node_id
        {
            enum
            {
#               define X(base,name,...) name,
#               include "ir_nodes.def"
#               undef X
                nnodes
            };
        };

        class args
        {
            std::vector<word> _args;

            void add() { }

            template<class... T> void add(word x, T... rest)
            {
                _args.push_back(x);
                add(rest...);
            }

        public:

            template<class... ARGS> args(ARGS... a)
            {
                add(a...);
            }

            word operator[](unsigned k) const
            {
                return _args[k];
            }

            word &operator[](unsigned k)
            {
                while (k >= _args.size()) _args.push_back(0);
                return _args[k];
            }

            word n() const
            {
                return _args.size();
            }
        };

        // only to be used as argument types in ir_nodes.def
        struct nothing { };
        struct nodeid { };

        template<class... ARGS> struct node_args
        {
            word operator[](unsigned) const
            {
                // TODO: throw something sensible
                throw 0;
            }

            word &operator[](unsigned)
            {
                // TODO: throw something sensible
                throw 0;
            }

            word nargs() const
            {
                return 0;
            }
        };

        template<> struct node_args<nothing>
        {
            node_args() { }

            word operator[](unsigned k) const
            {
                throw 0; // TODO: ...
            }

            word &operator[](unsigned k)
            {
                throw 0; // TODO: ...
            }

            word nargs() const
            {
                return 0;
            }

            static bool is_id(unsigned)
            {
                return false;
            }
        };

        template<> struct node_args<word>
        {
            word _last;

            node_args() { }

            node_args(word last) : _last(last) { }

            word operator[](unsigned k) const
            {
                // TODO: throw something if k != 0
                return _last;
            }

            word &operator[](unsigned k)
            {
                // TODO: throw something if k != 0
                return _last;
            }

            static bool is_id(unsigned k)
            {
                return false;
            }

            word nargs() const
            {
                return 1;
            }
        };

        template<> struct node_args<nodeid> : node_args<word>
        {
            node_args() { }

            node_args(word last) : node_args<word>(last) { }

            static bool is_id(unsigned k)
            {
                return true;
            }
        };

        template<> struct node_args<args>
        {
            args _args;

            node_args() { }

            template<class... ARGS> node_args(ARGS... a) : _args(a...) { }

            word operator[](unsigned k) const
            {
                return _args[k];
            }

            word &operator[](unsigned k)
            {
                return _args[k];
            }

            static bool is_id(unsigned k)
            {
                return true;
            }

            word nargs() const
            {
                return _args.n();
            }
        };

        template<class... ARGS> struct node_args<word, ARGS...>
        {
            word _first;
            node_args<ARGS...> _rest;

            node_args() { }

            template<class... REST> node_args(word first, REST... rest) : _first(first), _rest(rest...) { }

            word operator[](unsigned k) const
            {
                return k? _rest[k - 1] : _first;
            }

            word &operator[](unsigned k)
            {
                return k? _rest[k - 1] : _first;
            }

            static bool is_id(unsigned k)
            {
                return k? node_args<ARGS...>::is_id(k - 1) : false;
            }

            word nargs() const
            {
                return 1 + _rest.nargs();
            }
        };

        template<class... ARGS> struct node_args<nodeid, ARGS...> : node_args<word, ARGS...>
        {
            node_args() { }

            template<class... REST> node_args(word first, REST... rest) : node_args<word, ARGS...>(first, rest...) { }

            static bool is_id(unsigned k)
            {
                return k? node_args<ARGS...>::is_id(k - 1) : true;
            }
        };

        struct node
        {
            bool is_pure() const
            {
                return false;
            }
        };

        struct arithmetic : node
        {
            bool is_pure() const
            {
                return true;
            }
        };

        struct compare : node
        {
            bool is_pure() const
            {
                return true;
            }
        };

        struct typecon : node
        {
            bool is_pure() const
            {
                return true;
            }
        };

#       define X(base,name_,...) \
        class name_ : public base \
        { \
            node_args<__VA_ARGS__> _args; \
        public: \
            template<class... ARGS> name_(ARGS... args) : _args(args...) { } \
            name_(const buffer &buf, word &pos, word nargs) { for (word i = 0; i < nargs; ++i) (*this)[i] = buf.read(pos); } \
            word id() const { return node_id::name_; } \
            word operator[](unsigned k) const { return _args[k]; } \
            word &operator[](unsigned k) { return _args[k]; } \
            word nargs() const { return _args.nargs(); } \
            std::string name() const { return #name_; } \
            bool is_id(unsigned k) const { return node_args<__VA_ARGS__>::is_id(k); } \
        };
#       include "ir_nodes.def"

        struct vnode
        {
            virtual word id() const = 0;

            virtual word operator[](unsigned k) const = 0;

            virtual word nargs() const = 0;

            virtual std::string name() const = 0;

            virtual bool is_id(unsigned k) const = 0;

            virtual bool is_pure() const = 0;
        };

        template<class NODE> class vnode_impl : public vnode
        {
            const NODE _node;

        public:

            vnode_impl(const NODE &node) : _node(node) { }

            word id() const
            {
                return _node.id();
            }

            word operator[](unsigned k) const
            {
                return _node[k];
            }

            word nargs() const
            {
                return _node.nargs();
            }

            std::string name() const
            {
                return _node.name();
            }

            bool is_id(unsigned k) const
            {
                return _node.is_id(k);
            }

            bool is_pure() const
            {
                return _node.is_pure();
            }
        };

        class code;

        struct skip_query
        {
            using rval = int;

            template<class NODE> rval operator()(const code &, word, const NODE &node) const { }
        };

        struct vnode_query
        {
            using rval = vnode *;

            template<class NODE> rval operator()(const code &, word, const NODE &node) const
            {
                return new vnode_impl<NODE>(node);
            }
        };

        class code
        {
            buffer _buf;

        public:

            template<class NODE> word operator()(const NODE &node)
            {
                word pos = _buf.size();
                _buf.write(node.nargs());
                _buf.write(node.id());
                for (unsigned i = 0; i < node.nargs(); ++i) _buf.write(node[i]);
                _buf.write(_buf.size() - pos);
                return pos;
            }

            word operator()(char x) { return (*this)(Imm(x)); }
            word operator()(unsigned char x) { return (*this)(Imm(x)); }
            word operator()(short x) { return (*this)(Imm(x)); }
            word operator()(unsigned short x) { return (*this)(Imm(x)); }
            word operator()(int x) { return (*this)(Imm(x)); }
            word operator()(unsigned x) { return (*this)(Imm(x)); }
            word operator()(long x) { return (*this)(Imm(x)); }
            word operator()(unsigned long x) { return (*this)(Imm(x)); }
            word operator()(long long x) { return (*this)(Imm(x)); }
            word operator()(unsigned long long x) { return (*this)(Imm(x)); }

            template<class F> auto query(const F &f, word &index) const -> typename F::rval
            {
                word pos = index;
                word nargs = _buf.read(index);
                typename F::rval r;
                switch (_buf.read(index))
                {
#               define X(base,name,...) case node_id::name: \
                    r = f(*this, pos, name(_buf, index, nargs)); \
                    break;
#               include "ir_nodes.def"
                    default:
                        // TODO: throw something
                        ;
                }
                _buf.read(index);
                return r;
            }

            template<class F> auto query_at(const F &f, word index) const -> typename F::rval
            {
                return query(f, index);
            }

            vnode *read(word &index) const
            {
                return query(vnode_query(), index);
            }

            vnode *read_at(word index) const
            {
                return read(index);
            }

            bool next(word &index) const
            {
                query(skip_query(), index);
                return index < size();
            }

            template<class F> void pass(F &f, word &index) const
            {
                word pos = index;
                word nargs = _buf.read(index);
                switch (_buf.read(index))
                {
#               define X(base,name,...) case node_id::name: \
                    f(*this, pos, name(_buf, index, nargs)); \
                    break;
#               include "ir_nodes.def"
                    default:
                        // TODO: throw something
                        ;
                }
                _buf.read(index);
            }

            template<class F> void pass(F &f) const
            {
                for (word index = 0; index < _buf.size(); pass(f, index));
            }

            template<class F> void pass_at(F f, word index) const
            {
                return pass(f, index);
            }

            word arg(word index, word k) const
            {
                _buf.read(index);
                _buf.read(index);
                word x = _buf.read(index);
                while (k--) x = _buf.read(index);
                return x;
            }

            word size() const
            {
                return _buf.size();
            }

            void clear()
            {
                _buf.clear();
            }

        private:

            std::string node_text(word pos, const std::set<word> &onedef, std::map<word, word> &symmap, unsigned depth = 0) const
            {
                std::stringstream ss;
                auto &n = *read_at(pos);
                ss << "[ ";
                auto sym = symmap.find(pos);
                if (sym != symmap.end()) ss << n.name() << "_" << sym->second << ": ";
                ss << n.name() << std::endl;
                for (unsigned i = 0; i < n.nargs(); ++i)
                {
                    ss << std::string(depth * 2 + 2, ' ');
                    if (n.is_id(i))
                        if (onedef.count(n[i])) ss << node_text(n[i], onedef, symmap, depth + 1);
                        else
                        {
                            auto r = read_at(n[i]);
                            ss << r->name() << "_" << symmap[n[i]] << std::endl;
                            delete r;
                        }
                    else
                        ss << n[i] << std::endl;
                }
                ss << std::string(depth * 2, ' ') << ']' << std::endl;
                delete &n;
                return ss.str();
            }

        public:

            std::string text() const
            {
                std::stringstream ss;
                std::set<word> onedef;
                std::map<word, word> symmap;
                std::map<word, word> idmap;
                for (word pos = 0; pos < size(); )
                {
                    auto &n = *read(pos);
                    for (unsigned i = 0; i < n.nargs(); ++i)
                        if (n.is_id(i) && !symmap.count(n[i]))
                        {
                            auto r = read_at(n[i]);
                            if (r->is_pure())
                            {
                                auto od = onedef.find(n[i]);
                                if (od == onedef.end())
                                {
                                    onedef.insert(n[i]);
                                    delete r;
                                    continue;
                                }
                                else onedef.erase(od);
                            }
                            word id;
                            if (idmap.count(r->id())) id = idmap[r->id()]++;
                            else idmap[r->id()] = id = 0;
                            symmap[n[i]] = id;
                            delete r;
                        }
                    delete &n;
                }
                for (word pos = 0; pos < size(); next(pos) ) if (!onedef.count(pos)) ss << node_text(pos, onedef, symmap);
                return ss.str();
            }
        };

        class debug_writer
        {
            std::stringstream ss;

        public:

            template<class NODE> void operator()(const code &c, word index, const NODE &node)
            {
                ss.width(19);
                ss << std::right << index << " " << node.name();
                ss << std::string(16 - node.name().length(), ' ');
                for (word i = 0; i < node.nargs(); )
                {
                    ss << std::right << node[i];
                    if (++i < node.nargs()) ss << ", ";
                }
                ss << std::endl;
            }

            std::string str() const
            {
                return ss.str();
            }
        };
    }
}

#endif
