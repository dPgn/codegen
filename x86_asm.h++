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

#ifndef CODEGEN_X86_ASM_H
#define CODEGEN_X86_ASM_H

#include "module.h++"

namespace codegen
{
	namespace x86
	{
		class exception
		{
		};

		class argument_mismatch : public exception
		{
		};

		struct model
		{
			byte _bits = 64;

			bool operator==(const model &other) const
			{
				return _bits == other._bits;
			}

			bool operator!=(const model &other) const
			{
				return !(*this == other);
			}
		};

		struct operand
		{
		};

		class reg : public operand
		{
			byte _b, _i;

		public:

			reg() { }

			constexpr reg(byte b, byte i) : _b(b), _i(i) { }

			constexpr reg(const reg &r) : _b(r._b), _i(r._i) { }

			byte log2bits() const
			{
				return _b;
			}

			byte index() const
			{
				return _i;
			}

			bool is_integer() const
			{
				return false;
			}
		};

		struct integer_reg : reg
		{
			integer_reg() { }

			constexpr integer_reg(const integer_reg &r) : reg(r) { }

			constexpr integer_reg(byte b, byte i) : reg(b, i) { }

			bool is_integer() const
			{
				return true;
			}
		};

		static constexpr integer_reg RAX(6, 0);
		static constexpr integer_reg RCX(6, 1);
		static constexpr integer_reg RDX(6, 2);
		static constexpr integer_reg RBX(6, 3);
		static constexpr integer_reg RSP(6, 4);
		static constexpr integer_reg RBP(6, 5);
		static constexpr integer_reg RSI(6, 6);
		static constexpr integer_reg RDI(6, 7);

		static constexpr integer_reg R8(6, 8);
		static constexpr integer_reg R9(6, 9);
		static constexpr integer_reg R10(6, 10);
		static constexpr integer_reg R11(6, 11);
		static constexpr integer_reg R12(6, 12);
		static constexpr integer_reg R13(6, 13);
		static constexpr integer_reg R14(6, 14);
		static constexpr integer_reg R15(6, 15);

		static constexpr integer_reg EAX(5, 0);
		static constexpr integer_reg ECX(5, 1);
		static constexpr integer_reg EDX(5, 2);
		static constexpr integer_reg EBX(5, 3);
		static constexpr integer_reg ESP(5, 4);
		static constexpr integer_reg EBP(5, 5);
		static constexpr integer_reg ESI(5, 6);
		static constexpr integer_reg EDI(5, 7);

		static constexpr integer_reg R8D(5, 8);
		static constexpr integer_reg R9D(5, 9);
		static constexpr integer_reg R10D(5, 10);
		static constexpr integer_reg R11D(5, 11);
		static constexpr integer_reg R12D(5, 12);
		static constexpr integer_reg R13D(5, 13);
		static constexpr integer_reg R14D(5, 14);
		static constexpr integer_reg R15D(5, 15);

		static constexpr integer_reg AX(4, 0);
		static constexpr integer_reg CX(4, 1);
		static constexpr integer_reg DX(4, 2);
		static constexpr integer_reg BX(4, 3);
		static constexpr integer_reg SP(4, 4);
		static constexpr integer_reg BP(4, 5);
		static constexpr integer_reg SI(4, 6);
		static constexpr integer_reg DI(4, 7);

		static constexpr integer_reg R8W(4, 8);
		static constexpr integer_reg R9W(4, 9);
		static constexpr integer_reg R10W(4, 10);
		static constexpr integer_reg R11W(4, 11);
		static constexpr integer_reg R12W(4, 12);
		static constexpr integer_reg R13W(4, 13);
		static constexpr integer_reg R14W(4, 14);
		static constexpr integer_reg R15W(4, 15);

		static constexpr integer_reg AL(3, 0);
		static constexpr integer_reg CL(3, 1);
		static constexpr integer_reg DL(3, 2);
		static constexpr integer_reg BL(3, 3);

		// TODO: AH/CH/DH/BH and SPL/BPL/SIL/DIL

		static constexpr integer_reg R8L(3, 8);
		static constexpr integer_reg R9L(3, 9);
		static constexpr integer_reg R10L(3, 10);
		static constexpr integer_reg R11L(3, 11);
		static constexpr integer_reg R12L(3, 12);
		static constexpr integer_reg R13L(3, 13);
		static constexpr integer_reg R14L(3, 14);
		static constexpr integer_reg R15L(3, 15);

		struct not_a_label : exception
		{
		};

		struct unrelated_label : exception
		{
		};

		class assembler;

		struct symbol : operand
		{
			virtual std::int64_t addr() const = 0;

			virtual bool is_relative() const = 0;

			virtual byte nbytes() const = 0;

			virtual void mark(assembler &a)
			{
				throw not_a_label();
			}

			virtual symbol *clone() const = 0;
		};

		template<class T> class immediate : public symbol
		{
			const T _value;

		public:

			immediate(T value) : _value(value) { }

			std::int64_t addr() const // a bit of a misnomer here, perhaps rename?
			{
				return _value;
			}

			bool is_relative() const
			{
				return false;
			}

			byte nbytes() const
			{
				return sizeof(T);
			}

			symbol *clone() const
			{
				return new immediate(_value);
			}
		};

		class global : public symbol
		{
			std::int32_t _addr;

		public:

			std::int64_t addr() const
			{
				return _addr;
			}

 			bool is_relative() const // what was the point of this, again?
			{
				return true;
			}

			byte nbytes() const
			{
				return 4;
			}

			symbol *clone() const
			{
				return nullptr; // TODO: !!!!!
			}
		};

		class reg_mem : public operand
		{
			byte _index = 0;
			byte _index_log2bits;
			byte _index_shift;
			byte _base = 0;
			byte _base_log2bits;
			byte _seg;
			bool _has_index;
			bool _has_base;
			bool _indirect;
			byte _log2bits;

			symbol *_displacement = nullptr;

		public:

			reg_mem() { }

			reg_mem(const reg &r)
			{
				_index = r.index();
				_log2bits = _index_log2bits = r.log2bits();
				_indirect = false;
			}

			reg_mem(byte seg, byte width, const integer_reg &r, byte shift, const symbol *displacement = nullptr)
			{
				_index_shift = shift;
				if (r.index() != 4)
				{
					_index = r.index();
					_index_log2bits = r.log2bits();
					_has_index = true;
					_has_base = false;
				}
				else
				{
					_base = r.index();
					_base_log2bits = r.log2bits();
					_has_base = true;
					_has_index = false;
				}
				_indirect = true;
				_log2bits = width;
				if (displacement) _displacement = displacement->clone();
			}

			reg_mem(byte seg, byte width, const integer_reg &r, const symbol *displacement = nullptr)
				: reg_mem(seg, width, r, 0, displacement) { }

			reg_mem(byte seg, byte width, const integer_reg &r, byte shift, const integer_reg &b, const symbol *displacement = nullptr)
			{
				_index = r.index();
				_index_log2bits = r.log2bits();
				_index_shift = shift;
				_base = b.index();
				_base_log2bits = r.log2bits();
				_has_index = true;
				_has_base = true;
				_indirect = true;
				_log2bits = width;
				if (displacement) _displacement = displacement->clone();
			}

			reg_mem(byte seg, byte width, const integer_reg &r, const integer_reg &b, const symbol *displacement = nullptr)
				: reg_mem(seg, width, r, 0, b, displacement) { }

			reg_mem(byte seg, byte width, const symbol *displacement)
			{
				_has_index = false;
				_has_base = false;
				_log2bits = width;
				_displacement = displacement->clone();
			}

			bool is_indirect() const
			{
				return _indirect;
			}

			bool has_sib() const
			{
				return _indirect && (_index_shift || _has_base || _has_index && _index == 12);
			}

			byte modrm() const
			{
				if (!_indirect) return 0xc0 | _index & 7;
				byte nbytes = _displacement? _displacement->nbytes() : 0;
				if (nbytes > 4) throw argument_mismatch(); // TODO: better exception
				if (has_sib())
				{
					if (nbytes > 1) return 0x84;
					return nbytes? 0x44 : 4;
				}
				else if (_has_index)
				{
					if (nbytes > 1) return 0x80 | _index & 7;
					return (nbytes || (_index & 7) == 5? 0x40 : 0) | _index & 7;
				}
				else return nbytes? 5 : 4;
			}

			byte displacement_bytes() const
			{
				byte n = _displacement? _displacement->nbytes() : 0;
				return  n? n : _has_index && (_index & 7) == 5? 1 : 0;
			}

			void write_displacement(std::vector<byte> &code, const model &m) const
			{
				auto b = displacement_bytes();
				if (!b) return;
				auto addr = _displacement? _displacement->addr() : 0;
				for (byte i = 0; i < b; ++i) code.push_back((addr >> i * 8) & 0xff);
			}

			byte sib() const
			{
				return _index_shift << 6 | (_has_index? _index & 7 : 4) << 3 | _base & 7;
			}

			byte rex() const
			{
				byte x = 0;
				if (_has_base && _base & 8) x |= 0x41;
				if (_has_index && _index & 8) x |= 0x42;
				if (log2bits() == 6) x |= 0x48;
				return x;
			}

			byte log2bits() const
			{
				return _log2bits;
			}

			byte reg_index() const
			{
				return _index;
			}
		};

		class reg_imm : public operand
		{
			symbol *_immediate = nullptr;
			integer_reg _reg;

		public:

			reg_imm() { }

			reg_imm(const integer_reg &reg) : _reg(reg) { }

			reg_imm(symbol *imm) : _immediate(imm) { }

			reg_imm(const reg_imm &x) : _immediate(x._immediate? x._immediate->clone() : nullptr), _reg(x._reg) { }

			reg_imm(const reg_imm &&x) : _immediate(x._immediate), _reg(x._reg) { }

			~reg_imm()
			{
				if (_immediate) delete _immediate;
			}

			reg_imm &operator=(const integer_reg &reg)
			{
				if (_immediate) delete _immediate;
				_immediate = nullptr;
				_reg = reg;
				return *this;
			}

			reg_imm &operator=(std::int64_t value)
			{
				if (_immediate) delete _immediate;
				if (value > 0x7fffffffLL || value < -0x80000000LL) _immediate = new immediate<std::int64_t>(value);
				else if (value > 0x7fffL || value < -0x8000L) _immediate = new immediate<std::int32_t>(value);
				else if (value > 0x7fL || value < -0x80L) _immediate = new immediate<std::int16_t>(value);
				else  _immediate = new immediate<std::int8_t>(value);
				return *this;
			}

			reg_imm &operator=(const reg_imm &x)
			{
				if (_immediate) delete _immediate;
				_immediate = x._immediate? x._immediate->clone() : nullptr;
				_reg = x._reg;
				return *this;
			}

			reg_imm &operator=(const reg_imm &&x)
			{
				if (_immediate) delete _immediate;
				_immediate = x._immediate;
				_reg = x._reg;
				return *this;
			}

			bool is_immediate() const
			{
				return !!_immediate;
			}

			byte log2bits() const
			{
				if (!_immediate) return _reg.log2bits();
				auto n = _immediate->nbytes();
				return n > 4? 6 : n > 2? 5 : n > 1? 4 : 3;
			}

			byte nbytes() const
			{
				return _immediate? _immediate->nbytes() : 0;
			}

			byte rex() const
			{
				return _immediate? 0 : ((_reg.index() >> 3) & 1? 0x44 : 0) | (log2bits() == 6? 0x48 : 0);
			}

			byte modrm() const
			{
				return _immediate? 0 : (_reg.index() & 7) << 3;
			}

			void write_immediate(std::vector<byte> &code, const model &m, byte n) const
			{
				std::int64_t value = _immediate? _immediate->addr() : 0;
				for (byte i = 0; i < n; ++i) code.push_back((value >> i * 8) & 0xff);
			}
		};

		struct segment_reg : reg
		{
			struct segment_reg_and_width : reg
			{
				const byte _log2bits;

				segment_reg_and_width() : _log2bits(0) { }

				constexpr segment_reg_and_width(const segment_reg &r, byte width) : reg(r), _log2bits(width) { }

				constexpr segment_reg_and_width(byte i, byte width) : reg(4, i), _log2bits(width) { }

				reg_mem operator[](const integer_reg &r) const
				{
					return reg_mem(index(), _log2bits, r);
				}

				reg_mem operator[](const symbol &addr) const
				{
					return reg_mem(index(), _log2bits, &addr);
				}
			};

			segment_reg_and_width QWORD;
			segment_reg_and_width DWORD;
			segment_reg_and_width WORD;
			segment_reg_and_width BYTE;

			segment_reg() { }

			constexpr segment_reg(const segment_reg &r) : reg(r), QWORD(r, 6), DWORD(r, 5), WORD(r, 4), BYTE(r, 3) { }

			constexpr segment_reg(byte i) : reg(4, i), QWORD(*this, 6), DWORD(*this, 5), WORD(*this, 4), BYTE(*this, 3) { }

			reg_mem operator[](const integer_reg &r) const
			{
				return reg_mem(index(), 0, r);
			}

			reg_mem operator[](const symbol &addr) const
			{
				return reg_mem(index(), 0, &addr);
			}
		};

		static constexpr segment_reg ES(0);
		static constexpr segment_reg CS(1);
		static constexpr segment_reg SS(2);
		static constexpr segment_reg DS(3);
		static constexpr segment_reg FS(4);
		static constexpr segment_reg GS(5);

		template<byte W> struct simd_reg : reg
		{
			simd_reg() { }

			constexpr simd_reg(const simd_reg &r) : reg(r) { }

			constexpr simd_reg(byte i) : reg(W, i) { }
		};

		using sse_reg = simd_reg<7>;

		static constexpr sse_reg XMM0(0);
		static constexpr sse_reg XMM1(1);
		static constexpr sse_reg XMM2(2);
		static constexpr sse_reg XMM3(3);
		static constexpr sse_reg XMM4(4);
		static constexpr sse_reg XMM5(5);
		static constexpr sse_reg XMM6(6);
		static constexpr sse_reg XMM7(7);

		static constexpr sse_reg XMM8(8);
		static constexpr sse_reg XMM9(9);
		static constexpr sse_reg XMM10(10);
		static constexpr sse_reg XMM11(11);
		static constexpr sse_reg XMM12(12);
		static constexpr sse_reg XMM13(13);
		static constexpr sse_reg XMM14(14);
		static constexpr sse_reg XMM15(15);

		struct instruction
		{
			virtual instruction *clone() const = 0;

			virtual void encode(std::vector<byte> &code, const model &m) const = 0;

			virtual std::size_t length(const model &m) const
			{
				// TODO: override in derived classes to do this more efficiently
				std::vector<byte> temp;
				encode(temp, m);
				return temp.size();
			}
		};

		template<unsigned N, byte... C> class basic_instruction : public instruction
		{
			byte seq[N];

			void init(std::size_t k) { }

			template<class... B> void init(std::size_t k, byte c, B... rest)
			{
				seq[k] = c;
				init(k + 1, rest...);
			}

		public:

			basic_instruction()
			{
				init(0, C...);
			}

			instruction *clone() const
			{
				return new basic_instruction();
			}

			void encode(std::vector<byte> &code, const model &m) const
			{
				code.insert(code.end(), std::begin(seq), std::end(seq));
			}

			std::size_t length(const model &m) const
			{
				return N;
			}
		};

		using RET = basic_instruction<1, 0xc3>;

		using CLC = basic_instruction<1, 0xf8>;
		using CLI = basic_instruction<1, 0xfa>;
		using CLD = basic_instruction<1, 0xfc>;

		using STC = basic_instruction<1, 0xf9>;
		using STI = basic_instruction<1, 0xfb>;
		using STD = basic_instruction<1, 0xfd>;

		// just added the interrupt instructions for no reason; NOT TESTED!!!!
		// TODO: test (somehow)

		using INT1 = basic_instruction<1, 0xf1>;
		using INT3 = basic_instruction<1, 0xcc>;
		using INTO = basic_instruction<1, 0xce>;

		class INT : instruction
		{
			byte _k;

		public:

			INT(byte k) : _k(k) { }

			instruction *clone() const
			{
				return new INT(_k);
			}

			std::size_t length() const
			{
				return  _k == 3? 1 : 2;
			}

			void encode(std::vector<byte> &code, const model &m) const
			{
				if (_k == 3) // use INT3 as a shorter encoding for INT 3 like "real" assemblers
				{
					code.push_back(0xcc);
					return;
				}
				code.push_back(0xcd);
				code.push_back(_k);
			}
		};

		template<byte C> class basic_binary_integer_instruction : public instruction
		{
			byte _opcode = C;
			reg_mem _reg_mem;
			reg_imm _reg_imm;

		public:

			basic_binary_integer_instruction() { }

			basic_binary_integer_instruction(const integer_reg &dst, const integer_reg &src)
			{
				_reg_mem = dst;
				_reg_imm = src;
			}

			basic_binary_integer_instruction(const reg_mem &dst, const integer_reg &src)
			{
				_reg_mem = dst;
				_reg_imm = src;
			}

			basic_binary_integer_instruction(const integer_reg &dst, const reg_mem &src)
			{
				_reg_imm = dst;
				_reg_mem = src;
				_opcode = C | 2;
			}

			basic_binary_integer_instruction(const reg_mem &dst, std::int64_t src)
			{
				_reg_mem = dst;
				_reg_imm = src;
			}

			basic_binary_integer_instruction(const integer_reg &dst, std::int64_t src)
			{
				_reg_mem = dst;
				_reg_imm = src;
			}

			instruction *clone() const
			{
				basic_binary_integer_instruction *instr = new basic_binary_integer_instruction();
				instr->_reg_mem = _reg_mem;
				instr->_reg_imm = _reg_imm;
				instr->_opcode = _opcode;
				return instr;
			}

			void encode(std::vector<byte> &code, const model &m) const
			{
				byte log2bits = _reg_imm.is_immediate()? _reg_mem.log2bits() : _reg_imm.log2bits();
				if (m._bits != 16 && log2bits == 4) code.push_back(0x66);
				byte rex = _reg_mem.rex() | _reg_imm.rex();
				byte modrm = _reg_mem.modrm() | _reg_imm.modrm();
				byte immbytes = _reg_imm.nbytes();
				if (rex) code.push_back(rex);
				if (_reg_imm.is_immediate())
				{
					if (immbytes > 4) throw argument_mismatch();
					if (immbytes > 1 << _reg_mem.log2bits() - 3) throw argument_mismatch();
					if (!_reg_mem.is_indirect() && !_reg_mem.reg_index() && (_reg_mem.log2bits() < 5 || immbytes > 1)) // the shorter encodings for xAX
					{
						code.push_back(_opcode | 4 | (_reg_mem.log2bits() == 3? 1 : 0));
						immbytes = 1 << _reg_mem.log2bits() - 3;
					}
					else
					{
						if (_reg_mem.log2bits() == 3)
						{
							code.push_back(0x80);
							immbytes = 1;
						}
						else if (immbytes > 1)
						{
							code.push_back(0x81);
							immbytes = 1 << _reg_mem.log2bits() - 3;
						}
						else
						{
							code.push_back(0x83);
							immbytes = 1;
						}
						code.push_back(_opcode | modrm);
					}
				}
				else
				{
					if (!_reg_mem.is_indirect() && _reg_mem.log2bits() != _reg_imm.log2bits()) throw argument_mismatch();
					code.push_back(_reg_imm.log2bits() == 3? _opcode : _opcode | 1);
					code.push_back(modrm);
				}
				if (_reg_mem.has_sib()) code.push_back(_reg_mem.sib());
				_reg_mem.write_displacement(code, m);
				_reg_imm.write_immediate(code, m, immbytes);
			}
		};

		using MOV = basic_binary_integer_instruction<0x88>; // TODO: MOV needs its own class if we want to support all encodings

		using ADD = basic_binary_integer_instruction<0x00>;
		using OR  = basic_binary_integer_instruction<0x08>;
		using ADC = basic_binary_integer_instruction<0x10>;
		using SBB = basic_binary_integer_instruction<0x18>;
		using AND = basic_binary_integer_instruction<0x20>;
		using SUB = basic_binary_integer_instruction<0x28>;
		using XOR = basic_binary_integer_instruction<0x30>;
		using CMP = basic_binary_integer_instruction<0x38>;

		template<class T> class data_instruction : public instruction // not really an instruction but whatever
		{
			symbol *_data;

		public:

			data_instruction(const symbol &sym) : _data(sym.clone()) { }

			data_instruction(T data) : _data(new immediate<T>(data)) { }

			~data_instruction()
			{
				delete _data;
			}

			instruction *clone() const
			{
				return new data_instruction(*_data);
			}

			void encode(std::vector<byte> &code, const model &m) const
			{
				for (unsigned i = 0; i < sizeof(T); ++i) code.push_back((_data->addr() >> i * 8) & 0xff);
			}
		};

		using DB = data_instruction<std::int8_t>;
		using DW = data_instruction<std::int16_t>;
		using DD = data_instruction<std::int32_t>;
		using DQ = data_instruction<std::int64_t>;

		class assembler
		{
			// This is actually an overkill, as labels cannot work locally over section boundaries,
			// since we don't know where the linker will put them.
			struct section
			{
				std::vector<instruction *> _code;
				std::map<std::size_t, std::size_t> _label2pos;
				std::map<std::size_t, std::size_t> _pos2label;

				void clear()
				{
					for (auto i : _code) delete i;
					_code.clear();
					_pos2label.clear();
					_label2pos.clear();
				}
			}
			_text, _data, _bss;

			section *_section = &_text;

			std::map<std::size_t, model> _model = { { 0, model() } };
			std::size_t _next_label_index = 0;

			std::map<std::size_t, std::size_t> _label2addr;

			void compute_label_addresses()
			{
				// TODO: there are certainly more efficient ways to do this
				std::size_t prev = 1, total;
				_label2addr.clear();
				for (total = 0; total != prev; prev = total)
				{
					auto label = _text._pos2label.begin();
					auto nm = _model.begin(), m = nm++;
					for (std::size_t i = 0; i < _text._code.size() && label != _text._pos2label.end(); ++i)
					{
						if (nm != _model.end() && nm->first <= i) m = nm++;
						// TODO: align here
						if (i == label->first) _label2addr[label++->second] = total;
						total += _text._code[i]->length(m->second);
					}
				}
			}

		public:

			void operator()(const instruction &i)
			{
				_section->_code.push_back(i.clone());
			}

			void operator()(symbol &sym)
			{
				sym.mark(*this);
			}

			template<class T> function_module<T> assemble_function()
			{
				compute_label_addresses();

				std::vector<byte> buf;
				auto nm = _model.begin(), m = nm++;
				for (std::size_t i = 0; i < _text._code.size(); ++i)
				{
					if (nm != _model.end() && nm->first <= i) m = nm++;
					_text._code[i]->encode(buf, m->second);
				}
//				std::cout << std::endl;
//				for (auto b : buf) std::cout << std::hex << (int)b << std::endl;
				return function_module<T>(buf);
			}

			void text(model m = model())
			{
				_section = &_text;
				if ((--_model.upper_bound(_text._code.size()))->second != m) _model[_text._code.size()] = m;
			}

			void data()
			{
				_section = &_data;
			}

			void bss()
			{
				_section = &_bss;
			}

			void clear()
			{
				_model.clear();
				_model[0] = model();
				_text.clear();
				_data.clear();
				_bss.clear();
			}

			std::size_t new_label()
			{
				return _next_label_index++;
			}

			void mark_label(std::size_t index)
			{
				_section->_label2pos[index] = _section->_code.size();
				_section->_pos2label[_section->_code.size()] = index;
			}

			std::int64_t get_label_addr(std::size_t index)
			{
				return _label2addr[index];
			}
		};

		class label : public symbol
		{
			assembler *_a;
			std::size_t _index;

		public:

			label(assembler &a) : _a(&a)
			{
				_index = a.new_label();
			}

			std::int64_t addr() const
			{
				return _a->get_label_addr(_index);
			}

			byte nbytes() const
			{
				return 4;
			}

			bool is_relative() const
			{
				return true;
			}

			void mark(assembler &a)
			{
				if (&a != _a) throw unrelated_label();
				_a->mark_label(_index);
			}

			symbol *clone() const
			{
				return new label(*this);
			}
		};

		template<byte C> class branch_instruction : public instruction
		{
			label _target;

		public:

			branch_instruction(label target) : _target(target) { }

			instruction *clone() const
			{
				return new branch_instruction(_target);
			}

			void encode(std::vector<byte> &code, const model &m) const
			{
				auto addr = _target.addr() - code.size() - 2;
				if (addr > 127 || addr < -128)
				{
					code.push_back(0x70 | C);
					code.push_back(addr);
				}
				else
				{
					addr -= 4;
					code.push_back(0xf);
					code.push_back(0x80 | C);
					for (int i = 0; i < 4; addr >>= 8, ++i) code.push_back(0xff & addr);
				}
			}
		};

		using JO = branch_instruction<0>;
		using JNO = branch_instruction<1>;
		using JB = branch_instruction<2>;
		using JC = branch_instruction<2>;
		using JNAE = branch_instruction<2>;
		using JAE = branch_instruction<3>;
		using JNB = branch_instruction<3>;
		using JNC = branch_instruction<3>;
		using JE = branch_instruction<4>;
		using JZ = branch_instruction<4>;
		using JNE = branch_instruction<5>;
		using JNZ = branch_instruction<5>;
		using JBE = branch_instruction<6>;
		using JNA = branch_instruction<6>;
		using JNBE = branch_instruction<7>;
		using JA = branch_instruction<7>;
		using JS = branch_instruction<8>;
		using JNS = branch_instruction<9>;
		using JP = branch_instruction<10>;
		using JPE = branch_instruction<10>;
		using JNP = branch_instruction<11>;
		using JPO = branch_instruction<11>;
		using JL = branch_instruction<12>;
		using JNGE = branch_instruction<12>;
		using JGE = branch_instruction<13>;
		using JNL = branch_instruction<13>;
		using JLE = branch_instruction<14>;
		using JNG = branch_instruction<14>;
		using JG = branch_instruction<15>;
		using JNLE = branch_instruction<15>;

		template<bool CALL> class jump_instruction : public instruction
		{
			symbol *_target;

		public:

			jump_instruction(const symbol &target) : _target(target.clone()) { }

			jump_instruction(const jump_instruction &jmp) : _target(jmp._target->clone()) { }

			jump_instruction(const jump_instruction &&jmp) : _target(jmp._target) { }

			~jump_instruction()
			{
				delete _target;
			}

			jump_instruction &operator=(const jump_instruction &jmp)
			{
				delete _target;
				_target = jmp._target->clone();
			}

			jump_instruction &&operator=(const jump_instruction &&jmp)
			{
				delete _target;
				_target = jmp._target;
			}

			instruction *clone() const
			{
				return new jump_instruction(*this);
			}

			void encode(std::vector<byte> &code, const model &m) const
			{
				if (_target->is_relative())
				{
					auto addr = _target->addr() - code.size() - 2;
					if (!CALL && addr >= -128 && addr <= 127)
					{
						code.push_back(0xeb);
						code.push_back((byte)addr);
					}
					else
					{
						addr -= 3;
						code.push_back(CALL? 0xe8 : 0xe9);
						for (int i = 0; i < 4; addr >>= 8, ++i) code.push_back((byte)addr);
					}
				}
			}
		};

		using JMP = jump_instruction<false>;
		using CALL = jump_instruction<true>;

		template<byte C> class multiply_instruction : public instruction
		{
			byte _opcode = C << 3;

		protected:

			reg_mem _operand;

		public:

			multiply_instruction(const reg_mem &operand) : _operand(operand) { }

			multiply_instruction(const integer_reg &reg) : _operand(reg) { }

			instruction *clone() const
			{
				return new multiply_instruction(_operand);
			}

			void encode(std::vector<byte> &code, const model &m) const
			{
				if (m._bits != 16 && _operand.log2bits() == 4) code.push_back(0x66);
				byte rex = _operand.rex();
				if (rex) code.push_back(rex);
				code.push_back(_operand.log2bits() == 3? 0xf6 : 0xf7);
				code.push_back(_operand.modrm() | _opcode);
				if (_operand.has_sib()) code.push_back(_operand.sib());
				_operand.write_displacement(code, m);
			}
		};

		using MUL = multiply_instruction<4>;
		using DIV = multiply_instruction<6>;
		using IDIV = multiply_instruction<7>;

		class IMUL : public multiply_instruction<5>
		{
			bool _is_ordinary = false;
			integer_reg _reg;
			symbol *_immediate = nullptr;

		public:

			IMUL(const reg_mem &operand) : multiply_instruction<5>(operand), _is_ordinary(true) { }

			IMUL(const integer_reg &reg) : multiply_instruction<5>(reg), _is_ordinary(true) { }

			IMUL(const integer_reg &x, const integer_reg &y) : multiply_instruction<5>(y), _reg(x) { }

			IMUL(const integer_reg &x, const reg_mem &y) : multiply_instruction<5>(y), _reg(x) { }

			IMUL(const integer_reg &x, const reg_mem &y, std::int32_t imm) : multiply_instruction<5>(y), _reg(x)
			{
				if (imm > 0x7fffL || imm < -0x8000L) _immediate = new immediate<std::int32_t>(imm);
				else if (imm > 0x7f || imm < -0x80) _immediate = new immediate<std::int16_t>(imm);
				else _immediate = new immediate<byte>(imm);
			}

			IMUL(const integer_reg &x, const integer_reg &y, std::int32_t imm) : IMUL(x, reg_mem(y), imm) { }

			IMUL(const integer_reg &x, const reg_mem &y, const symbol &imm) : multiply_instruction<5>(y), _reg(x)
			{
				_immediate = imm.clone();
			}

			IMUL(const integer_reg &x, const integer_reg &y, const symbol &imm) : IMUL(x, reg_mem(y), imm) { }

			instruction *clone() const
			{
				if (_is_ordinary) return new multiply_instruction<5>(_operand);
				if (_immediate) return new IMUL(_reg, _operand, *_immediate);
				return new IMUL(_reg, _operand);
			}

			void encode(std::vector<byte> &code, const model &m) const
			{
				if (_is_ordinary)
				{
					multiply_instruction<5>::encode(code, m);
					return;
				}
				if (m._bits != 16 && _operand.log2bits() == 4) code.push_back(0x66);
				byte rex = _operand.rex();
				if (_reg.index() & 8) rex |= 0x44;
				if (rex) code.push_back(rex);
				if (_immediate) code.push_back(_immediate->nbytes() > 1? 0x69 : 0x6b);
				else
				{
					code.push_back(0x0f);
					code.push_back(0xaf);
				}
				code.push_back((_reg.index() & 7) << 3 | _operand.modrm());
				if (_operand.has_sib()) code.push_back(_operand.sib());
				_operand.write_displacement(code, m);
				if (_immediate)
				{
					auto addr = _immediate->addr();
					if (_immediate->nbytes() <= 1) code.push_back((byte)addr);
					else for (int i = 0; i < 4 && i < (1 << _operand.log2bits() - 3); addr >>= 8, ++i) code.push_back((byte)addr);
				}
			}
		};

		template<byte C> class barrel_instruction : public instruction
		{
			byte _opcode = C << 3;
			reg_mem _operand;
			byte _count; // I doubt we'll ever need to use a symbol as an argument to a shift
			bool _cl = false;

			barrel_instruction(const reg_mem &operand, byte count, bool cl) : _operand(operand), _count(count), _cl(cl) { }

		public:

			barrel_instruction(const reg_mem &operand, byte count) : _operand(operand), _count(count) { }

			barrel_instruction(const integer_reg &operand, byte count) : _operand(operand), _count(count) { }

			barrel_instruction(const reg_mem &operand, const integer_reg &must_be_cl) : _operand(operand), _cl(true)
			{
				if (must_be_cl.index() != 1 || must_be_cl.log2bits() != 3) throw argument_mismatch();
			}

			barrel_instruction(const integer_reg &operand, const integer_reg &must_be_cl)
				: barrel_instruction(reg_mem(operand), must_be_cl) { }

			instruction *clone() const
			{
				return new barrel_instruction(_operand, _count, _cl);
			}

			void encode(std::vector<byte> &code, const model &m) const
			{
				byte lsb = _operand.log2bits() > 3? 1 : 0;
				if (m._bits != 16 && _operand.log2bits() == 4) code.push_back(0x66);
				byte rex = _operand.rex();
				if (rex) code.push_back(rex);
				if (_cl) code.push_back(lsb | 0xd2);
				else if (_count == 1) code.push_back(lsb | 0xd0);
				else code.push_back(lsb | 0xc0);
				code.push_back(_opcode | _operand.modrm());
				if (_operand.has_sib()) code.push_back(_operand.sib());
				_operand.write_displacement(code, m);
				if (!_cl && _count != 1) code.push_back(_count);
			}
		};

		using ROL = barrel_instruction<0>;
		using ROR = barrel_instruction<1>;
		using RCL = barrel_instruction<2>;
		using RCR = barrel_instruction<3>;
		using SHL = barrel_instruction<4>;
		using SAL = barrel_instruction<4>;
		using SHR = barrel_instruction<5>;
		using SAR = barrel_instruction<7>;

		// this is for floating point SSE instructions; integer instructions need a slightly different logic
		template<byte P, byte C> class typical_sse_instruction : public instruction
		{
			byte _prefix = P; // 0 = none
			byte _opcode = C; // second opcode byte, first is 0F

			sse_reg _dst;
			reg_mem _src;

		public:

			typical_sse_instruction(const sse_reg &dst, const reg_mem &src) : _dst(dst), _src(src) { }

			typical_sse_instruction(const sse_reg &dst, const sse_reg &src) : typical_sse_instruction(dst, reg_mem(src)) { }

			instruction *clone() const
			{
				return new typical_sse_instruction(_dst, _src);
			}

			void encode(std::vector<byte> &code, const model &m) const
			{
				if (_prefix) code.push_back(_prefix);
				byte rex = _src.rex() | (_dst.index() & 8? 0x44 : 0);
				if (rex) code.push_back(rex);
				code.push_back(0xf);
				code.push_back(_opcode);
				code.push_back((_dst.index() & 7) << 3 | _src.modrm());
				if (_src.has_sib()) code.push_back(_src.sib());
				_src.write_displacement(code, m);
			}
		};

		using ADDPS = typical_sse_instruction<0, 0x58>;
		using ADDPD = typical_sse_instruction<0x66, 0x58>;
		using ADDSS = typical_sse_instruction<0xf3, 0x58>;
		using ADDSD = typical_sse_instruction<0xf2, 0x58>;
		using SUBPS = typical_sse_instruction<0, 0x5c>;
		using SUBPD = typical_sse_instruction<0x66, 0x5c>;
		using SUBSS = typical_sse_instruction<0xf3, 0x5c>;
		using SUBSD = typical_sse_instruction<0xf2, 0x5c>;
		using MULPS = typical_sse_instruction<0, 0x59>;
		using MULPD = typical_sse_instruction<0x66, 0x59>;
		using MULSS = typical_sse_instruction<0xf3, 0x59>;
		using MULSD = typical_sse_instruction<0xf2, 0x59>;
		using DIVPS = typical_sse_instruction<0, 0x5e>;
		using DIVPD = typical_sse_instruction<0x66, 0x5e>;
		using DIVSS = typical_sse_instruction<0xf3, 0x5e>;
		using DIVSD = typical_sse_instruction<0xf2, 0x5e>;
		using RCPPS = typical_sse_instruction<0, 0x53>;
		using RCPSS = typical_sse_instruction<0xf3, 0x53>;
		using RSQRTPS = typical_sse_instruction<0, 0x52>;
		using RSQRTSS = typical_sse_instruction<0xf3, 0x52>;
		using SQRTPS = typical_sse_instruction<0, 0x51>;
		using SQRTPD = typical_sse_instruction<0x66, 0x51>;
		using SQRTSS = typical_sse_instruction<0xf3, 0x51>;
		using SQRTSD = typical_sse_instruction<0xf2, 0x51>;
	}
}

#endif
