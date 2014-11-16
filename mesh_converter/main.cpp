#include <iostream>

#include <assimp/Importer.hpp>      // C++ importer interface
#include <assimp/scene.h>           // Output data structure
#include <assimp/postprocess.h>     // Post processing flags

#include "arc/io/FileStream.hpp"
#include "arc/io/SimpleMesh.hpp"
#include "arc/memory/util.hpp"
#include "arc/math/common.hpp"
#include "arc/hash/StringHash.hpp"
#include "arc/string/StringView.hpp"

int main(int argc, char** argv)
{
	using namespace arc;

	arc::String input_path = "../../icosphere.obj";
	arc::String output_path = "../../icoshphere_from_obj.sm.arc";

	// Create an instance of the Importer class
	Assimp::Importer importer;

	// And have it read the given file with some example postprocessing
	// Usually - if speed is not the most important aspect for you - you'll 
	// propably to request more postprocessing than we do in this example.
	const aiScene* scene = importer.ReadFile(std::string(input_path.c_str()),
		aiProcess_Triangulate |
		aiProcess_SortByPType);

	if (!scene)
	{
		std::cout << "[ERROR] Could not open input file: " << input_path.c_str() << std::endl;
		system("pause");
		return EXIT_FAILURE;
	}

	arc::io::FileWriteStream out;
	out.open(output_path);

	if (!out.is_open())
	{
		std::cout << "[ERROR] Could not open output file: " << output_path.c_str() << std::endl;
		system("pause");
		return EXIT_FAILURE;
	}

	// convert

	io::MeshHeader mh;
	mh.magic_number = io::MeshHeader::MAGIC;
	mh.version = io::MeshHeader::CURRENT_VERSION;
	mh.part_count = scene->mNumMeshes;
	out.write(&mh, sizeof(io::MeshHeader));

	uint32_t data_offset = sizeof(io::MeshHeader);
	data_offset += mh.part_count*sizeof(io::PartHeader);

	if (mh.part_count > 1024)
	{
		std::cout << "[ERROR] Too many parts: " << mh.part_count << std::endl;
		system("pause");
		return EXIT_FAILURE;
	}

	io::PartHeader parts[1024];

	for (uint32_t mi=0; mi < scene->mNumMeshes; mi++)
	{
		auto& mesh = *scene->mMeshes[mi];
		if (mesh.mPrimitiveTypes ^ aiPrimitiveType_TRIANGLE)
		{
			std::cout << "[ERROR] unsupported primitive type" << std::endl;
			system("pause");
			return EXIT_FAILURE;
		}
		if (!mesh.HasPositions())
		{
			std::cout << "[ERROR] input mesh has no position data" << std::endl;
			system("pause");
			return EXIT_FAILURE;
		}
		if (mesh.GetNumColorChannels() > 2)
		{
			std::cout << "[WARNING] input mesh has " << mesh.GetNumColorChannels() << " vertex color channels, only the first 2 will be converted" << std::endl;
		}
		if (mesh.GetNumUVChannels() > 2)
		{
			std::cout << "[WARNING] input mesh has " << mesh.GetNumUVChannels() << " uv channels, only the first 2 will be converted" << std::endl;
		}

		io::PartHeader& ph =  parts[mi];
		ph.data_offset = data_offset;
		ph.index_count = mesh.mNumFaces * 3;
		ph.vertex_count = mesh.mNumVertices;
		ph.index_type = static_cast<uint8_t>(
			mesh.mNumVertices <= std::numeric_limits<uint8_t>::max()  ? renderer::IndexType::Uint8 :
			mesh.mNumVertices <= std::numeric_limits<uint16_t>::max() ? renderer::IndexType::Uint16 : renderer::IndexType::Uint32);
		ph.primitive_type = static_cast<uint8_t>(renderer::PrimitiveType::Triangle);
		uint32_t name_len = static_cast<uint32_t>(min(sizeof(ph.name), mesh.mName.length + 1));
		memory::util::copy(mesh.mName.data, ph.name, name_len);
		ph.name_hash = string_hash64(StringView(ph.name, 0, name_len)).value();
		ph.attributes.color1 = mesh.GetNumColorChannels() >= 1;
		ph.attributes.color2 = mesh.GetNumColorChannels() >= 2;
		ph.attributes.normal = mesh.HasNormals();
		ph.attributes.uv1 = mesh.GetNumUVChannels() >= 1;
		ph.attributes.uv1 = mesh.GetNumUVChannels() >= 2;

		data_offset += io::vertex_data_size(ph) + io::index_data_size(ph);
		out.write(&ph, sizeof(ph));
	}

	for (uint32_t mi = 0; mi < scene->mNumMeshes; mi++)
	{
		auto& mesh = *scene->mMeshes[mi];
		auto& ph = parts[mi];

		auto dbg_a = out.tell();
		auto dbg_b = io::index_data_begin(ph);
		ARC_ASSERT(out.tell() == io::index_data_begin(ph), "Invalid file offset");
		
		// write index data
		if (ph.index_type == 1)
		{
			for (uint32_t i = 0; i < mesh.mNumFaces; i++)
			{
				uint8_t indices[3] = { 
					mesh.mFaces[i].mIndices[0], 
					mesh.mFaces[i].mIndices[1], 
					mesh.mFaces[i].mIndices[2] 
				};
				out.write(indices, sizeof(uint8_t) * 3);
			}
		}
		else if (ph.index_type == 2)
		{
			for (uint32_t i = 0; i < mesh.mNumFaces; i++)
			{
				uint16_t indices[3] = {
					mesh.mFaces[i].mIndices[0],
					mesh.mFaces[i].mIndices[1],
					mesh.mFaces[i].mIndices[2]
				};
				out.write(indices, sizeof(uint16_t) * 3);
			}
		}
		else 
		{
			for (uint32_t i = 0; i < mesh.mNumFaces; i++)
			{
				uint32_t indices[3] = {
					mesh.mFaces[i].mIndices[0],
					mesh.mFaces[i].mIndices[1],
					mesh.mFaces[i].mIndices[2]
				};
				out.write(indices, sizeof(uint32_t) * 3);
			}
		}

		ARC_ASSERT(out.tell() == io::vertex_data_begin(ph), "Invalid file offset");

		// write vertex data
		for (uint32_t i = 0; i < mesh.mNumVertices; i++)
		{
			out.write(&mesh.mVertices[i].x, 3 * sizeof(float));
			if (ph.attributes.normal) out.write(&mesh.mNormals[i].x, 3 * sizeof(float));
			if (ph.attributes.color1)
			{
				auto& c = mesh.mColors[0][i];
				uint8_t color[4] = { c.r*255.0f, c.g*255.0f, c.b*255.0f, c.a*255.0f };
				out.write(color, 4 * sizeof(uint8_t));
			}
			if (ph.attributes.color2)
			{
				auto& c = mesh.mColors[1][i];
				uint8_t color[4] = { c.r*255.0f, c.g*255.0f, c.b*255.0f, c.a*255.0f };
				out.write(color, 4 * sizeof(uint8_t));
			}
			if (ph.attributes.uv1) out.write(&mesh.mTextureCoords[0][i].x, 2 * sizeof(float));
			if (ph.attributes.uv2) out.write(&mesh.mTextureCoords[1][i].x, 2 * sizeof(float));
		}
	}

	std::cout << "[Success] Output written to: " << output_path.c_str() << std::endl;
	system("pause");
	return 0;
}

