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

		struct architecture
		{
			virtual unsigned bits() const = 0;
		};

		struct x64 : architecture
		{
			unsigned bits() const
			{
				return 64;
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

		class complex_operand : public operand
		{
			enum
			{
				_plain = 0, _indirect, _immediate
			}
			_mode = _plain;

			byte _r, _i = 4, _b, _s;
			std::int64_t _imm = 0;

		public:

			complex_operand() { }

			complex_operand(const reg &r)
			{
				_r = r.index();
				_b = r.log2bits();
			}

			complex_operand(byte seg, const integer_reg &r)
			{
				_r = r.index();
				_b = r.log2bits();
				_s = seg;
				_mode = _indirect;
			}

			complex_operand &operator =(const reg &r)
			{
				_r = r.index();
				_b = r.log2bits();
				_mode = _plain;
				return *this;
			}

			byte rm() const
			{
				return _r;
			}

			bool has_sib_index() const
			{
				return _i != 4;
			}

			byte sib_index() const
			{
				return _i;
			}

			std::int64_t imm() const
			{
				return _imm;
			}

			bool is_indirect() const
			{
				return _mode == _indirect;
			}

			bool is_plain() const
			{
				return _mode == _plain;
			}

			bool is_immediate() const
			{
				return _mode == _immediate;
			}

			byte log2bits() const
			{
				return _b;
			}
		};

		struct segment_reg : reg
		{
			segment_reg() { }

			constexpr segment_reg(const segment_reg &r) : reg(r) { }

			constexpr segment_reg(byte i) : reg(4, i) { }

			complex_operand operator[](const integer_reg &r) const
			{
				return complex_operand(index(), r);
			}
		};

		static constexpr segment_reg ES(0);
		static constexpr segment_reg CS(1);
		static constexpr segment_reg SS(2);
		static constexpr segment_reg DS(3);
		static constexpr segment_reg FS(4);
		static constexpr segment_reg GS(5);

		struct instruction
		{
			virtual instruction *clone() const = 0;

			virtual void encode(std::vector<byte> &code, const architecture &arch) const = 0;

			virtual std::size_t length(const architecture &arch) const = 0;
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

			void encode(std::vector<byte> &code, const architecture &arch) const
			{
				code.insert(code.end(), std::begin(seq), std::end(seq));
			}

			std::size_t length(const architecture &arch) const
			{
				return N;
			}
		};

		using RET = basic_instruction<1, 0xc3>;
		using STC = basic_instruction<1, 0xf9>;

		template<byte C> class basic_binary_integer_instruction : public instruction
		{
			bool dbit;
			integer_reg r;
			complex_operand ind;

		protected:

			byte get_rex() const
			{
				byte rex = 0;
				if (r.log2bits() == 6) rex |= 0x48;
				if (r.index() & 8) rex |= 0x44;
				if (ind.sib_index() & 8) rex |= 0x42;
				if (ind.rm() & 8) rex |= 0x41;
				return rex;
			}

		public:

			basic_binary_integer_instruction() { }

			basic_binary_integer_instruction(const integer_reg &dst, const integer_reg &src)
			{
				r = dst;
				ind = src;
				dbit = true;
				if (r.log2bits() != ind.log2bits()) throw argument_mismatch();
			}

			basic_binary_integer_instruction(const complex_operand &dst, const integer_reg &src)
			{
				r = src;
				ind = dst;
				dbit = false;
				if (r.log2bits() != ind.log2bits()) throw argument_mismatch();
			}

			basic_binary_integer_instruction(const integer_reg &dst, const complex_operand &src)
			{
				r = dst;
				ind = src;
				dbit = true;
				if (r.log2bits() != ind.log2bits()) throw argument_mismatch();
			}

			instruction *clone() const
			{
				basic_binary_integer_instruction *instr = new basic_binary_integer_instruction();
				instr->r = r;
				instr->ind = ind;
				instr->dbit = dbit;
				return instr;
			}

			// TODO: move the relevant stuff from here to complex_operand
			void encode(std::vector<byte> &code, const architecture &arch) const
			{
				if (arch.bits() >= 32 && r.log2bits() == 4 || arch.bits() == 16 && r.log2bits() == 5) code.push_back(0x66);
				// TODO: address size prefix (0x67)
				byte rex = get_rex();
				if (rex) code.push_back(rex);
				byte opcode = C;
				if (dbit) opcode |= 2;
				if (r.log2bits() > 3) opcode |= 1;
				code.push_back(opcode);
				int offset_bytes = 0;
				byte modrm = (r.index() & 7) << 3;
				if (ind.is_indirect())
				{
					if (ind.has_sib_index())
					{
					}
					else
					{
						if ((ind.rm() & 7) == 4)
						{
						}
						else
						{
							if (ind.imm() > 127 || ind.imm() < -128)
							{
							}
							else if (ind.imm() || (ind.rm() & 7) == 5)
							{
								modrm |= 0x40 | ind.rm() & 7;
								offset_bytes = 1;
							}
							else
							{
								modrm |= ind.rm() & 7;
							}
						}
					}
				}
				else
					modrm |= 0xc0 | ind.rm() & 7;
				code.push_back(modrm);
				for (int i = 0; i < offset_bytes; ++i) code.push_back(ind.imm() >> i * 8 & 0xff);
			}

			std::size_t length(const architecture &arch) const
			{
				// TODO: do this more efficiently if compilers turn out to actually create the temp vector
				std::vector<byte> temp;
				encode(temp, arch);
				return temp.size();
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

		struct assembler
		{
			std::vector<instruction *> _code;
			const architecture &_arch;

		public:

			assembler(const architecture &arch = x64()) : _arch(arch) { }

			void operator()(const instruction &i)
			{
				_code.push_back(i.clone());
			}

			template<class T> function_module<T> assemble_function()
			{
				std::vector<byte> buf;
				for (auto i : _code) i->encode(buf, _arch);
//				for (auto b : buf) std::cout << std::hex << (int)b << std::endl;
				return function_module<T>(buf);
			}

			void clear()
			{
				for (auto i : _code) delete i;
				_code.clear();
			}
		};
	}
}

#endif
