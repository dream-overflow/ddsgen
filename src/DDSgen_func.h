/**
 * @file DDSgen_func.h
 * @brief Extrait les param�tres de la ligne de commande et initialises la std::map.
 * @author Emmanuel RUFFIO (emmanuel.ruffio@gmail.com)
 * @date 2006-06-30
 * @copyright Copyright (c) 2001-2017 Dream Overflow. All rights reserved.
 * @details 
 */

#ifndef _O3D_DDSGEN_FUNC_H
#define _O3D_DDSGEN_FUNC_H

#include <map>
#include <string>
#include <sstream>

#include <o3d/core/String.h>

/**
 * @brief Extrait les param�tres de la ligne de commande et initialises la std::map.
 * La clef de la map correspond au caract�re (le param�tre) et la chaine � la valeur du
 * param�tre.
 * @author Emmanuel Ruffio
 * @date 2006-06-30
 * @pre _argc et _argv ne doivent pas avoir �t� modifi� depuis le d�marrage du
 *      programme.
 * @exception E_invalid_parameter lev�e si un param�tre est d�clar� deux fois
 *		      E_help_required lev�e si on d�tecte une requ�te d'aide
 *			  E_syntax_error lev�e si aucune valeur n'est associ�e � un param�tre
 */
void Extract_parameters(
				o3d::UInt32 _argc,
				o3d::Char ** _argv,
				std::map<o3d::Char, o3d::String> & _parameters);

/**
 * @brief V�rifies tous les arguments pr�sent dans la std::map en se basant sur une chaine
 *        qui r�pertorie tous les arguments valides (params[] definie dessous).
 * @pre aucune
 * @author Emmanuel Ruffio
 * @date 2006-06-30
 * @exception E_invalid_parameter lev�e si la valeur d'un argument n'est pas correct
 *		      E_unknown_parameter lev�e si un param�tre est utilis�, alors qu'il n'existe
 *		  			              pas dans la chaine params[].
 *            E_input_file_undefined lev�e si aucun fichier d'entr�e n'est d�finie, cad
 *                                   le param�tre 'i' vide.
 */
void Check_parameters(std::map<o3d::Char, o3d::String> & _parameters);

/**
 * @brief Retourne True si le param�tre _key est d�finie dans la std::map, False sinon
 * @author Emmanuel Ruffio
 * @date 2006-06-30
 */
o3d::Bool Defined(o3d::Char _key, std::map<o3d::Char, o3d::String> & _parameters);

//
// EXCEPTIONS
//
struct E_syntax_error
{
	E_syntax_error(o3d::String _msg = ""): msg(_msg) {}

	o3d::String msg;
};

struct E_invalid_parameter
{
	E_invalid_parameter(o3d::String _msg = ""): msg(_msg) {}

	o3d::String msg;
};

struct E_unknown_parameter
{
	E_unknown_parameter(o3d::String _msg = ""): msg(_msg) {}

	o3d::String msg;
};

struct E_input_file_undefined
{
	E_input_file_undefined(o3d::String _msg = ""): msg(_msg) {}

	o3d::String msg;
};

struct E_help_required
{
	E_help_required(o3d::String _msg = ""): msg(_msg) {}

	o3d::String msg;
};

static char msg_help[] = 
"DDSgen : Generates DDS file from various picture format.\n \
Parameters:\n \
  -h : help\n \
  -i <file> : input file\n \
  -o <file> : output file (optional)\n \
  -p <format> : pixel format (RGB, RGBA, BGR, BGRA, GREY) (default : RGB/RGBA/GREY)\n \
  -f <format> : dds format (DXT1/DXT3/DXT5/(argb)4444/1555/8888/(rgb)565/888/555)\n \
  -m <bool> : use mipmap (true/false)\n \
  -n <number>: max number of Mip maps to generate (default : 0 (0 means all mipmap))\n \
  -s <number-number> : splits the picture into n*m dds file (if possible) called <output file>_<n>_<m>\n";

static char params[] = "iopfmns";

#endif // _O3D_DDSGEN_FUNC_H
 