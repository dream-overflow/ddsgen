/**
 * @file DDSgen.cpp
 * @brief Main entry.
 * @author Emmanuel RUFFIO (emmanuel.ruffio@gmail.com)
 * @date 2006-06-30
 * @copyright Copyright (c) 2001-2017 Dream Overflow. All rights reserved.
 * @details 
 */

#include <iostream>
#include <fstream>

#include <dxtlib/dxtlib.h>
#include "DDSgen_func.h"

#include <o3d/core/FileManager.h>
#include <o3d/core/Debug.h>
#include <o3d/image/Image.h>

using namespace o3d;

// CallBack appelé par la fonction nvDXTcompress pour la sortie sur fichier
NV_ERROR_CODE DDS_write(const void *buffer, size_t count, const MIPMapData * mipMapData, void * userData)
{
	OutStream * file = (OutStream*)userData;

	UInt32 res = file->writer((const char*)buffer, 1, (std::streamsize)count);

	return res == count ? NV_OK : NV_READ_FAILED;
}

// Main entry
int main(int argc, char *argv[])
{
	if (argc == 1)
	{
		std::cout << "Usage: [options] [input] [output]" << std::endl;
		std::cout << "-h for help" << std::endl;
		return 0;
	}

	// Initialisation du gestionnaire d'erreur
	Debug::instance()->getLogger().setLogFileName("ddsgen.log");

	// Analyse des paramètres d'entrée
	std::map<Char, String> parameters;

	try
	{
		Extract_parameters(argc, argv, parameters);
	}
	catch(const E_syntax_error & _err)
	{ std::cout << "Error : Syntax" << std::endl; return 1; }
	catch(const E_invalid_parameter & _err)
	{ std::cout << "Error : " << _err.msg.getData() << std::endl; return 1;	}
	catch(const E_help_required & _err)
	{ std::cout << msg_help << std::endl; return 0;	}
	catch(...)
	{ std::cout << "Undefined error" << std::endl; return 1; }

	try
	{
		Check_parameters(parameters);
	}
	catch(const E_unknown_parameter & _err)
	{ std::cout << "Error : Unknown parameter " << _err.msg.getData() << std::endl;	return 1; }
	catch(const E_input_file_undefined & _err)
	{ std::cout << "Error : No input file defined" << std::endl; return 1; }

	// Ouverture des fichiers d'entrée et sortie
	String infile_name(parameters['i']);		// Nom du fichier d'entrée
	String outfile_name(parameters['o']);		// Nom du fichier de sortie
	String outfile_without_ext = outfile_name;
	{
		UInt32 ext = outfile_without_ext.reverseFind('.');
		if (ext != -1)
			outfile_without_ext.truncate(ext);
	}

	Image infile;   			// Objet image de O3D
	FileOutStream *outfile;		// Objet fichier de O3D

	try
	{
		infile.load(infile_name);
	}
	catch(E_BaseException &e)
	{
		return 1;
	}

	if (!Defined('s', parameters))
 		return 1
	
	try
	{
		outfile = FileManager::instance()->openOutStream(outfile_name, FileOutStream::CREATE);
	}
	catch(E_BaseException &e)
	{
		return 1;
	}

	// Initialisation des paramètres par défaut
	// Initialisation du format de pixel en entrée
	nvPixelOrder param_pixel_order;

	if (!Defined('p', parameters))
		switch(infile.getBpp())
		{
			case 1: param_pixel_order = nvGREY; break;
			case 3: param_pixel_order = nvRGB; break;
			case 4: param_pixel_order = nvRGBA; break;
			default: std::cout << "Unable to determine pixel format (RGBA/BGRA, RGB/BGR, GREY)" << std::endl; return 1; break;
		}
	else
	{
		if (((parameters['p'] == String("RGBA")) || (parameters['p'] == String("BGRA"))) && (infile.getBpp() != 4))
		{
			std::cout << "Pixel format incompatible with the input file" << std::endl;
			return 1;
		}
		else if (((parameters['p'] == String("RGB")) || (parameters['p'] == String("BGR"))) && (infile.getBpp() != 3))
		{
			std::cout << "Pixel format incompatible with the input file" << std::endl;
			return 1;
		}
		else if ((parameters['p'] == String("GREY")) && (infile.getBpp() != 1))
		{
			std::cout << "Pixel format incompatible with the input file" << std::endl;
			return 1;
		}

		if (parameters['p'] == String("RGB")) param_pixel_order = nvRGB;
		if (parameters['p'] == String("RGBA")) param_pixel_order = nvRGBA;
		if (parameters['p'] == String("BGR")) param_pixel_order = nvBGR;
		if (parameters['p'] == String("BGRA")) param_pixel_order = nvBGRA;
		if (parameters['p'] == String("GREY")) param_pixel_order = nvGREY;
	}

	// Initialisation du format de texture en sortie
	nvTextureFormats param_texture_format;

	if (!Defined('f', parameters))
	{
		std::cout << "Texture format undefined" << std::endl;
		return 1;
	}

	if (parameters['f'] == String("DXT1"))
		if (infile.getBpp() == 4) param_texture_format = kDXT1a;
		else param_texture_format = kDXT1;
	if (parameters['f'] == String("DXT3")) param_texture_format = kDXT3;
	if (parameters['f'] == String("DXT5")) param_texture_format = kDXT5;
	if (parameters['f'] == tring("4444")) param_texture_format = k4444;
	if (parameters['f'] == String("1555")) param_texture_format = k1555;
	if (parameters['f'] == String("8888")) param_texture_format = k8888;
	if (parameters['f'] == String("565")) param_texture_format = k565;
	if (parameters['f'] == String("888")) param_texture_format = k888;
	if (parameters['f'] == String("555")) param_texture_format = k555;

	// Génération ou non des mipmaps
	nvMipMapGeneration param_use_mipmap = kNoMipMaps;
	if (Defined('m', parameters))
	{
		if (parameters['m'] == O3DString("true")) param_use_mipmap = kGenerateMipMaps;
		else param_use_mipmap = kNoMipMaps;
	}

	// Nombre de mipmaps à générer
	UInt32 param_mipmap_number = 0;
	if (Defined('n', parameters))
	{
		std::istringstream iss(std::string(parameters['n'].getData()));
		iss >> param_mipmap_number;
	}

	// Paramètrage du découpage (optionnel) de l'image d'entrée
	Bool param_split = Defined('s', parameters);;
	UInt32 param_split_x = 1;
	UInt32 param_split_y = 1;

	if (param_split)
	{
		UInt32 score = parameters['s'].find('-');

		if ((score == -1) || (score == 0) || (score == parameters['s'].length() - 1))
		{
			std::cout << "Invalid parameter ('s') value ( -s n-m )" << std::endl;
			return 1;
		}

		String first = parameters['s'];
		first.Truncate(score);

		String second = parameters['s'].sub(score + 1);

		std::istringstream iss1(first.getData());
		std::istringstream iss2(second.getData());

		if ((!(iss1 >> param_split_x) || !iss1.eof()) || (!(iss2 >> param_split_y) || !iss2.eof()))
		{
			std::cout << first.getData() << "  " << second.getData() << std::endl;
			std::cout << "Invalid parameter ('s') value" << std::endl;
			return 1;
		}

		if ((param_split_x == 0 || param_split_y == 0) ||
			(!((infile.getWidth() + param_split_x - 1) % param_split_x == 0)) ||
			(!((infile.getHeight() + param_split_y - 1) % param_split_y == 0)))
		{
			std::cout << "Split impossible with those values" << std::endl;
			std::cout << "If you split the image in N*M tiles, remember that:" << std::endl;
			std::cout << "IMG_X + N - 1 modulo N = 0 (same for Y)" << std::endl;
			return 1;
		}
	}

	nvCompressionOptions options;
	options.SetDefaultOptions();
	options.textureFormat = param_texture_format;
	options.mipMapGeneration = param_use_mipmap;
	options.numMipMapsToWrite = param_mipmap_number;
	options.user_data = &outfile;

	// Fin de paramètrage des données
	NV_ERROR_CODE hres;

	if (!param_split)
	{
		hres = nvDDS::nvDXTcompress((unsigned char*)infile.getData(),
												  infile.getWidth(),
												  infile.getHeight(),
												  infile.getWidth()*infile.getBpp(),
												  param_pixel_order,
												  &options,
												  DDS_write,
												  NULL);

		if (hres == NV_OK) std::cout << "Success" << std::endl;
		else std::cout << "Error : Code " << (Int32)hres << std::endl;

		infile.destroy();
		deletePtr(outfile);

    	return 0;
	}
	else
	{
		UInt32 tile_x = infile.getWidth() / param_split_x;
		UInt32 tile_y = infile.getHeight() / param_split_y;

		if (param_split_x == 1) tile_x--;
		if (param_split_y == 1) tile_y--;

		unsigned char * buffer = new unsigned char[(tile_x+1)*(tile_y+1)*infile.getBpp()];

		for (UInt32 j = 0; j < param_split_y ; j++)
		{
			for (UInt32 i = 0; i < param_split_x ; i++)
			{
				// Creation du fichier de destination
				String new_filename;// = outfile_without_ext;
				new_filename << i+1 << '_' << param_split_y - j << '_' << outfile_without_ext << String(".dds");

				try {				
					outfile = FileManager::instance()->openOutStream(new_filename, FileOutStream::CREATE);
				}
				catch (E_BaseException &e)
				{
						std::cout << "Impossible to open/create the file : " << new_filename.getData() << std::endl;
						std::cout << "Tous les fichiers DDS n'ont pas été généré" << std::endl;
						return 1;
				}

				// Creation du buffer
				for (UInt32 y = j * tile_y; y <= (j+1) * tile_y ; y++)
				{
					memcpy(
									&buffer[(y-j * tile_y) * (tile_x + 1)*infile.getBpp()],
									&infile.getData()[y * infile.getWidth() * infile.getBpp() + i * tile_x * infile.GetBpp()],
									(tile_x + 1) * infile.getBpp());
				}

				hres = nvDDS::nvDXTcompress((unsigned char*)buffer,
														  tile_x+1,
														  tile_y+1,
														  (tile_x+1)*infile.getBpp(),
														  param_pixel_order,
														  &options,
														  DDS_write,
														  NULL);

				if (hres == NV_OK)
				{
					std::cout << "Output file created." << std::endl;
				}
				else
				{
					std::cout << "Error during output file creation. Code : " << (Int32)hres << std::endl;
					if ((Int32)hres == -24)
					{
						std::cout << "This error is probably due to an incorrect texture format." << std::endl;
						std::cout << "If you want to use DXTi format, the tiles must be 2^k x 2^l" << std::endl;
					}
				}

				deletePtr(outfile);
			}
		}

		deleteArray(buffer);
		infile.destroy();

		return 0;
	}

	return 1;
}
