--[[

	inkZ80 Table Generator using Lua 5.2

	Written by Mark Incley

--]]


-- Constants
FLAG_S 		= 0x80
FLAG_Z 		= 0x40
FLAG_Bit5 	= 0x20
FLAG_H	 	= 0x10
FLAG_Bit3	= 0x08
FLAG_P 		= 0x04
FLAG_V		= FLAG_P
FLAG_N 		= 0x02
FLAG_C 		= 0x01

outfile = "tables.cpp"
bytesPerLine = 16


-- Bit test function
function bittest(v, p)
  return v % (p + p) >= p
end


function MakeTables()
	local	v, b, p

	-- Tables
	ZS = {}
	ZS35 = {}
	INC8 = {}
	DEC8 = {}
	Parity = {}
	ZS35P = {}

	-- 8-bit Increment and Decrement
	for v=0,255 do
		bf = 0
		p = true

		-- Common INC/DEC flags
		if v >= 128 then bf = bf + FLAG_S end
		if v == 0 then bf = bf + FLAG_Z end
		ZS[v] = bf

		-- Undocumented: bits 3 and 5 of the result peek through!
		if bittest(v, FLAG_Bit5) then bf = bf + FLAG_Bit5 end
		if bittest(v, FLAG_Bit3) then bf = bf + FLAG_Bit3 end

		-- INC flags
		i = bf
		if v == 0x80 then i = i + FLAG_V end
		if v % 0x10 == 0 then i = i + FLAG_H end

		-- DEC flags
		d = bf
		d = d + FLAG_N
		if v == 0x7f then d = d + FLAG_V end
		if v % 0x10 == 0xf then d = d + FLAG_H end

		-- Parity
		for b=0,7 do
			if bittest(v, 2^b) then p = not p end
		end

		if p then p = FLAG_P else p = 0 end
		Parity[v] = p
		ZS35[v] = bf
		ZS35P[v] = bf + p
		INC8[v] = i
		DEC8[v] = d
--~ 		print(string.format("0x%02x: ZS35 = 0x%02x,  INC = 0x%02x, DEC = 0x%02x, Parity = %d", v, bf, i, d, p))
	end
end


function WriteTable(f, tab, label, decimal)
	bytes = #tab + 1	-- Lua is only counting from [1]
	print(string.format("Writing table %s (%d entries)", label, bytes))
	f:write(string.format("static const BYTE %s[%d] =\n{\n", label, bytes))
	-- Write column index
	f:write("//   ")
	for i=0,bytesPerLine-1 do
		f:write(string.format("+%02x   ", i))
	end
	-- Write the table contents
	i = 0;
	while i < bytes do
		if i % bytesPerLine == 0 then
			f:write("\n\t") else f:write(", ")
		end
		if decimal then
			f:write(string.format(" % 3d", tab[i]))
		else
			f:write(string.format("0x%02x", tab[i]))
		end
		i = i + 1
		if i % bytesPerLine == 0 then
			if i < bytes then
				f:write(",")
			end
			f:write(string.format("\t\t\t// %02x - %02x", i-bytesPerLine, i-1))
		end
	end
	f:write("\n};\n\n")
end


function WriteTables()
	print(string.format("Writing to \"%s\"", outfile))
	f = io.open(outfile, "w")
	f:write("// inkZ80 Tables Produced by maketab.lua. Do not modify directly as changes may be overwritten!\n\n")
	WriteTable(f, Parity, 		"tableParity",			false)
	WriteTable(f, ZS, 			"tableZS",				false)
	WriteTable(f, ZS35, 		"tableZS35",			false)
	WriteTable(f, ZS35P, 		"tableZS35P",			false)
	WriteTable(f, INC8, 		"tableZSHV35_INC8",		false)
	WriteTable(f, DEC8,		  	"tableZSHV35_DEC8",		false)
	f:close()
end


MakeTables()
WriteTables()
print("Done.")
