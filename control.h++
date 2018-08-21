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

#ifndef CODEGEN_CONTROL_H
#define CODEGEN_CONTROL_H

#include "remapper.h++"

namespace codegen
{
    namespace control
    {
        template<class OUT> struct structurizer
        {
            std::map<ir::word, unsigned> _loops;
            std::map<ir::word, std::vector<ir::word>> _skips;
            std::set<ir::word> _labels;
            remapper<OUT> _out;

            struct loop_entry
            {
                ir::word _label;
                ir::word _forever;
            };

            std::vector<loop_entry> _loop_stack;

            struct loop_counter
            {
                structurizer &_s;
                std::set<ir::word> _marked;

                loop_counter(structurizer &s) : _s(s) { }

                void operator()(const ir::code &code, ir::word pos, const ir::node &node) { }

                void add_loop(ir::word pos, ir::word label)
                {
                    if (!_marked.count(label)) return; // TODO: make this a reverse pass, in which case we must remove the negation from the condition
                    auto cnt = _s._loops.find(label);
                    if (cnt == _s._loops.end()) _s._loops.insert({ label, 1 });
                    else ++cnt->second;
                }

                void operator()(const ir::code &code, ir::word pos, const ir::Jump &node)
                {
                    add_loop(pos, node[0]);
                }

                void operator()(const ir::code &code, ir::word pos, const ir::Branch &node)
                {
                    add_loop(pos, node[0]);
                }

                void operator()(const ir::code &code, ir::word pos, const ir::Mark &node)
                {
                    _marked.insert(node[0]);
                }
            };

            structurizer(OUT &out) : _out(remapper<OUT>(out)) { }

            template<class NODE> void end_loop(const NODE &node)
            {
                if (!--_loops[node[0]]) _labels.erase(node[0]);
                if (_loops[node[0]] || node[0] != _loop_stack.back()._label) _skips[node[0]].push_back(_out.add(ir::Skip()));
                while (!_loop_stack.empty() && !_loops[_loop_stack.back()._label])
                {
                    for (auto skip = _skips[node[0]].rbegin(); skip != _skips[node[0]].rend(); ++skip) _out(ir::Here(*skip));
                    _out(ir::Repeat(_loop_stack.back()._forever));
                    _loop_stack.pop_back();
                }
            }

            template<class NODE> void operator()(const ir::code &code, ir::word pos, const NODE &node)
            {
                _out(pos, node);
            }

            void operator()(const ir::code &code, ir::word pos, const ir::Label &node) { }

            void operator()(const ir::code &code, ir::word pos, const ir::Branch &node)
            {
                if (!_labels.count(node[0])) _skips[node[0]].push_back(_out(pos, ir::SkipIf(node[1])));
                else
                {
                    // As the optimizer should be able to handle things like this, I make no
                    // attempt to avoid generaring excessive code here.
                    auto negated = _out.add(ir::Not(node[1]));
                    auto skip = _out.add(ir::SkipIf(negated));
                    end_loop(node);
                    _out(ir::Here(skip));
                }
            }

            void operator()(const ir::code &code, ir::word pos, const ir::Jump &node)
            {
                if (!_labels.count(node[0])) _skips[node[0]].push_back(_out(pos, ir::Skip()));
                else end_loop(node);
            }

            void operator()(const ir::code &code, ir::word pos, const ir::Mark &node)
            {
                for (auto skip = _skips[node[0]].rbegin(); skip != _skips[node[0]].rend(); ++skip) _out(ir::Here(*skip));
                _labels.insert(node[0]);
                if (_loops.count(node[0])) _loop_stack.push_back({ node[0], _out(pos, ir::Forever()) });
            }
        };

        template<class OUT> struct unstructurizer
        {
            remapper<OUT> _out;

            unstructurizer(OUT &out) : _out(remapper<OUT>(out)) { }

            template<class NODE> void operator()(const ir::code &code, ir::word pos, const NODE &node)
            {
                _out(pos, node);
            }

            void operator()(const ir::code &code, ir::word pos, const ir::Forever &node)
            {
                _out(ir::Mark(_out(pos, ir::Label())));
            }

            void operator()(const ir::code &code, ir::word pos, const ir::Repeat &node)
            {
                _out(ir::Jump(node[0]));
            }

            void operator()(const ir::code &code, ir::word pos, const ir::Skip &node)
            {
                _out(pos, ir::Label());
                _out(ir::Jump(pos));
            }

            void operator()(const ir::code &code, ir::word pos, const ir::SkipIf &node)
            {
                _out(pos, ir::Label());
                _out(ir::Branch(pos, node[0]));
            }

            void operator()(const ir::code &code, ir::word pos, const ir::Here &node)
            {
                _out(ir::Mark(node[0]));
            }
        };

        template<class OUT> void structurize(OUT &out, const ir::code &code)
        {
            structurizer<OUT> s(out);
            typename structurizer<OUT>::loop_counter lc(s);
            code.pass(lc);
            code.pass(s);
        }

        template<class OUT> OUT structurized(const ir::code &code)
        {
            OUT out;
            structurize(out, code);
            return out;
        }

        template<class OUT> void unstructurize(OUT &out, const ir::code &code)
        {
            unstructurizer<OUT> u(out);
            code.pass(u);
        }

        template<class OUT> OUT unstructurized(const ir::code &code)
        {
            OUT out;
            unstructurize(out, code);
            return out;
        }
    }
}

#endif
