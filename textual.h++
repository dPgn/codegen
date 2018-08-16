#include "ir.h++"

#include <unordered_map>

namespace codegen
{
    class textual
    {
        std::unordered_map<std::string, ir::word> _symtable;
        ir::code _code;

    public:

        template<class IT> struct syntax_error
        {
            IT _pos;
        };

        template<class IT> struct undefined_symbol
        {
            std::string _sym;
            IT _pos;
        };

    protected:

        template<class IT> void skip_whitespace(IT &it, const IT &end)
        {
            for (;;)
            {
                while (it != end && *it <= ' ') ++it;
                if (it != end && *it == '#')
                    while (it != end && *it != '\r' && *it != '\n') ++it;
                else break;
            }
        }

        template<class IT> std::string get_sym(IT &it, const IT &end)
        {
            IT start = it;
            while (it != end && *it > ' ' && *it != ']' && *it != '[' && *it != ':' && *it != '"' && *it != '#') ++it;
            return std::string(start, it);
        }

        template<class IT> std::int_least64_t get_number(const std::string &num, IT &it)
        {
            int pos = 0;
            int sign = num[0] == '-'? -++pos : 1;
            std::int_least64_t x = 0;
            while (pos < num.length())
                if (num[pos] < '0' || num[pos] > '9') throw syntax_error<IT> { it };
                else x = x * 10 + num[pos++] - '0';
            return x * sign;
        }

        template<class IT> void handle_string(IT &it, const IT &end)
        {
            ir::Str node;
            IT start = it;
            ++it;
            while (it != end && *it != '"')
            {
                unsigned c = 0;
                if (*it == '\\')
                    if (++it == end) throw syntax_error<IT> { it };
                    else if (*it == '"') node[node.nargs()] = '"';
                    else if (*it == '\\') node[node.nargs()] = '\\';
                    else
                    {
                        while (it != end && *it >= '0' && *it <= '7') c = (c << 3) | (*it++ - '0');
                        if (it == end || *it != '\\') throw syntax_error<IT> { it };
                        ++it;
                        node[node.nargs()] = (char)c;
                    }
                else node[node.nargs()] = *it++;
            }
            if (it == end) throw syntax_error<IT> { start };
            ++it;
            _code(node);
        }

        template<class IT, class NODE> void handle(IT &it, const IT &end)
        {
            NODE node;
            unsigned k = 0;
            while (it != end && *it != ']')
            {
                if (*it == '"')
                {
                    node[k++] = _code.size();
                    handle_string(it, end);
                }
                else if (*it == '-' || *it >= '0' && *it <= '9')
                {
                    node[k++] = get_number(get_sym(it, end), it);
                }
                else if (*it == '[')
                {
                    node[k++] = _code.size();
                    handle_block(++it, end);
                }
                else
                {
                    auto start = it;
                    auto symstr = get_sym(it, end);
                    auto sym = _symtable.find(symstr);
                    if (sym == _symtable.end()) throw undefined_symbol<IT> { symstr, start };
                    node[k++] = sym->second;
                }
                skip_whitespace(it, end);
            }
            _code(node);
        }

        template<class IT> void handle_block(IT &it, const IT &end)
        {
            IT start = it;
            std::string sym;
            for (;;)
            {
                skip_whitespace(it, end);
                sym = get_sym(it, end);
                skip_whitespace(it, end);
                if (it == end || *it != ':') break;
                _symtable[sym] = _code.size();
                ++it;
            }
            if (sym[0] == '-' || sym[0] >= '0' && sym[0] <= '9') _code(ir::Imm(get_number(sym, it)));
#           define X(base,name,...) else if (sym == #name) handle<IT, ir::name>(it, end);
#           include "ir_nodes.def"
            else throw syntax_error<IT> { start };
            if (it == end || *it != ']') throw syntax_error<IT> { it };
            ++it;
        }

        template<class IT> void parse(IT &it, const IT &end)
        {
            skip_whitespace(it, end);
            while (it != end && *it == '[')
            {
                ++it;
                handle_block(it, end);
                skip_whitespace(it, end);
            }
            if (it != end) throw syntax_error<IT> { it };
        }

    public:

        textual(const std::string text)
        {
            auto it = text.begin();
            parse(it, text.end());
        }

        const ir::code &code() const
        {
            return _code;
        }
    };
}
