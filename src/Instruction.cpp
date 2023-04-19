#include <Instruction.h>

Instruction::Type Instruction::get_type() { return t; }

Fields Instruction::get_fields() { return f; }

IName Instruction::get_name() { return name; }
