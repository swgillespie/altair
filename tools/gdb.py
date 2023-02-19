# This file is a part of Altair, a chess engine.
# Copyright (C) 2017-2023 Sean Gillespie <sean@swgillespie.me>.
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <https://www.gnu.org/licenses/>.

import gdb
from gdb.printing import RegexpCollectionPrettyPrinter, PrettyPrinter, register_pretty_printer

class BitboardPrinter:
    """
    Pretty printer for bitboards.
    """
    val: gdb.Value

    def __init__(self, val: gdb.Value) -> None:
        self.val = val

    def to_string(self) -> str:
        bits = self.val["bits_"]
        out_str = "\n"
        for rank in range(7, -1, -1):
            for file in range(8):
                sq = rank * 8 + file
                if ((bits & (1 << sq)) != 0):
                    out_str += " 1 "
                else:
                    out_str += " . "
            
            out_str += f"| {rank+1}\n"
        
        for _ in range(8):
            out_str += "---"
        out_str += "\n"
        for i in range(8):
            out_str += f" {chr(i+97)} "
        out_str += "\n"
        return out_str

    def display_hint(self) -> None:
        return None

class ValuePrinter:
    """
    Printer for altair::Value.
    """
    VALUE_MATED = -32768 // 2 + 1
    VALUE_MATE = 32767 // 2
    MATE_DISTANCE_MAX = 50

    val: gdb.Value

    def __init__(self, val: gdb.Value) -> None:
        self.val = val

    def to_string(self) -> str:
        val = self.val["centipawns_"]
        if val > self.VALUE_MATE:
            return f"#{self.VALUE_MATE + self.MATE_DISTANCE_MAX - val}"
        elif val < self.VALUE_MATED:
            return f"#-{val - self.VALUE_MATED + self.MATE_DISTANCE_MAX}"
        return f"{val}"

    def display_hint(self) -> str:
        return 'string'

def load_altair_pretty_printers() -> PrettyPrinter:
    pp = RegexpCollectionPrettyPrinter("altair")
    pp.add_printer('Bitboard', '^altair::Bitboard$', BitboardPrinter)
    pp.add_printer('Value', '^altair::Value$', ValuePrinter)
    return pp

register_pretty_printer(gdb.current_objfile(), load_altair_pretty_printers())