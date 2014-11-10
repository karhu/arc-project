
function preprocess_config(config)
	for k,v in pairs(config.vertex.input) do
		v["type"] = v[1];
		v["gen_name"] = "_in_" .. v[2];
		v["is_float"] = v[1] <= 4;
		v["var_name"] = v[2];
	end

	for k,v in pairs(config.vertex.output) do
		v["type"] = v[1];
		v["gen_name"] = "_v_" .. v[2];
		v["var_name"] = v[2];
	end

	local i = 1

	for k,v in pairs(config.vertex.uniforms.instance) do
		v["type"] = v[1]
		v["gen_name"] = "_" .. string.gsub(v[2],"%.","_")
		v["var_name"] = v[2]
		v["binding"] = i
		i = i+1
	end
end

function read_shader_config(filepath)

	print(_VERSION)

    local config_context = {}        -- create new environment
    setmetatable(config_context, {__index = _G})
    setfenv(0, config_context)    -- set it

	local config = assert(loadfile(filepath))
	config()

	preprocess_config(config_context)

	return config_context
end

function generate_source_code(config)

	local context = {
		max_batch_size = 128; -- TODO compute this at runtime, for now this is a safe value
	}

	local vert_output = {}
	local frag_output = {}

	local function add_vert_output(s)
		table.insert(vert_output,s)
	end

	local function get_vert_output()
		return table.concat(vert_output)
	end

	local function add_frag_output(s)
		table.insert(frag_output,s)
	end

	local function get_frag_output()
		return table.concat(frag_output)
	end

	local vertex_source = config.vertex.main;
	local fragment_source = config.fragment.main

	-- header

	add_vert_output("// AUTOMATICALLY GENERATED SHADER \n")
	add_vert_output("// DO NOT EDIT \n\n")
	add_vert_output("#version " .. config.version .. "\n\n");

	add_frag_output("// AUTOMATICALLY GENERATED SHADER \n")
	add_frag_output("// DO NOT EDIT \n\n")
	add_frag_output("#version " .. config.version .. "\n\n");

	-- vertex uniforms
	add_vert_output("// Instance Uniforms \n")

	local tmp1 = "layout(std140, binding = "
	local tmp2 = ") uniform "
	local tmp3 = " { "
	local tmp4 = " };\n"

	for k,v in pairs(config.vertex.uniforms.instance) do
		local t = string_from_type[v.type]
		local var = t .. " " .. v.gen_name .. "[".. context.max_batch_size .."];"
		add_vert_output(tmp1 .. v.binding .. tmp2 .. v.gen_name .. "_T" .. tmp3 .. var .. tmp4)

		local indexed_name = v.gen_name .. "[_id]"

		vertex_source   = string.gsub(vertex_source,   v.var_name, indexed_name)
		fragment_source = string.gsub(fragment_source, v.var_name, indexed_name)
	end
	add_vert_output("\n")

	-- inputs
	add_vert_output("// Vertex inputs \n")
	for k,v in pairs(config.vertex.input) do
		local t = string_from_type[v.type]
		local var = "in " .. t .. " " .. v.gen_name .. ";\n"

		-- modify vertex source
		local old = "in.".. v.var_name;
		local new = v.gen_name
		vertex_source = string.gsub(vertex_source,old,new)

		add_vert_output(var);
	end
	add_vert_output("\n")

	-- vertex outputs aka fragment inputs
	add_vert_output("// Vertex outputs \n")
	add_frag_output("// Fragment inputs \n")
	for k,v in pairs(config.vertex.output) do
		-- vertex
		local t = string_from_type[v.type]
		local var = "out " .. t .. " " .. v.gen_name .. ";\n"

		-- modify vertex source
		local old = "out.".. v.var_name;
		local new = v.gen_name;
		vertex_source = string.gsub(vertex_source,old,new)

		add_vert_output(var);

		-- fragment
		local t = string_from_type[v.type]
		local var = "in " .. t .. " " .. v.gen_name .. ";\n"

		-- modify fragment source
		local old = "in.".. v.var_name;
		local new = v.gen_name;
		fragment_source = string.gsub(fragment_source,old,new)

		add_frag_output(var);
	end
	add_vert_output("\n")
	add_frag_output("\n")
	vertex_source = string.gsub(vertex_source,"out.projected","gl_Position")

	-- vertex main

	if (config.debug_vertex_pre_main) then
		add_vert_output("// debug begin\n")
		add_vert_output(config.debug_vertex_pre_main)
		add_vert_output("// debug end\n\n")
	end

	add_vert_output("// main\n")
	add_vert_output("void main()\n{\n")
	add_vert_output(vertex_source)
	add_vert_output("}\n")
	add_vert_output("\n")

	-- fragment main

	if (config.debug_fragment_pre_main) then
		add_frag_output("// debug begin\n")
		add_frag_output(config.debug_fragment_pre_main)
		add_frag_output("// debug end\n\n")
	end

	add_frag_output("// main\n")
	add_frag_output("void main()\n{\n")
	add_frag_output(fragment_source)
	add_frag_output("}\n")
	add_frag_output("\n")

	shader_name = "test"

	-- write .vert file
	generated_vert = get_vert_output();
	--local file,err = io.open(config.file_path..config.file_name..".vert","w")
	--if not file then return print(err) end
	--file:write(generated_vert)
	--file:close()

	-- write .frag file
	generated_frag = get_frag_output();
	--local file,err = io.open(config.file_path..config.file_name..".frag","w")
	--if not file then return print(err) end
	--file:write(generated_frag)
	--file:close()
	return {vertex = generated_vert, fragment = generated_frag}
end

--------------------------------------------
--     define a few necessary globals     --
--------------------------------------------

float = 1
vec2  = 2
vec3  = 3
vec4  = 4

mat2  = 5
mat3  = 6
mat4  = 7

uint   = 8
vec2u  = 9
vec3u  = 10
vec4u  = 11

string_from_type = {
	"float","vec2","vec3","vec4",
	"mat2","mat3","mat4",
	"uint","uvec2","uvec3","uvec4",	
}
