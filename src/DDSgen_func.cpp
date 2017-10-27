/**
 * @file DDSgen_func.cpp
 * @brief Extrait les paramètres de la ligne de commande et initialises la std::map.
 * @author Emmanuel RUFFIO (emmanuel.ruffio@gmail.com)
 * @date 2006-06-30
 * @copyright Copyright (c) 2001-2017 Dream Overflow. All rights reserved.
 * @details 
 */

#include "DDSgen_func.h"

#include <sstream>
#include <iostream>

using namespace o3d;

void Extract_parameters(UInt32 _argc, Char ** _argv, std::map<Char, String> & _parameters)
{
	std::vector<String> blocks;
	String param;
	UInt32 i = 1;

	while (i < _argc)
	{
		if (_argv[i][0] != '-') param = _argv[i];
		else
		{
			param = _argv[i];
			i++;

			if (i < _argc) param += _argv[i];
		}
		blocks.push_back(param);
		i++;
	}

	std::map<Char, String>::iterator it; // Itérateur permettant de faire toute sorte de recherche

	for (i = 0; i < blocks.size() ; i++)
	{
		if (blocks[i][0] != '-')
		{
			if ((blocks[i][0] == 'h') || (blocks[i][0] == '?') || (blocks[i] == String("help"))) throw E_help_required();

			// Il s'agit alors soit du fichier d'entrée, soit du fichier de sortie
			it = _parameters.find('i');

			if (it == _parameters.end()) _parameters['i'] = blocks[i];
			else
			{
				it = _parameters.find('o');
				if (it == _parameters.end()) _parameters['o'] = blocks[i];
				else throw E_invalid_parameter(String(blocks[i]));
			}
		}
		else if (blocks[i].Length() > 2) // au minimul: -k4 => 3 caratères
		{
			it = _parameters.find(blocks[i][1]);

			if (it != _parameters.end()) throw E_invalid_parameter(String("Parameter already defined"));
			else _parameters[blocks[i][1]] = blocks[i].GetSubString(2);
		}
		else if ((blocks[i].Length() == 2) && (blocks[i][1] == 'h')) throw E_help_required();
		else throw E_syntax_error();
	}
}

void Check_parameters(std::map<Char, String> & _parameters)
{
	Bool find;

	for (std::map<Char, String>::iterator it = _parameters.begin() ; it != _parameters.end() ; it++)
	{
		find = False;

		for (UInt32 i = 0 ; i < strlen(params) ; i++)
			if (params[i] == it->first)
			{
				switch(params[i])
				{
				case 'p' : if ((it->second != String("RGB")) && (it->second != String("BGR")) && (it->second != String("RGBA")) && (it->second != String("BGRA")) && (it->second != String("GREY")))
							throw E_invalid_parameter(String("p"));
					break;
				case 'f' : if ((it->second != String("DXT1")) && 
							 (it->second != String("DXT3")) && 
							 (it->second != String("DXT5")) && 
							 (it->second != String("4444")) && 
							 (it->second != String("1555")) && 
							 (it->second != String("8888")) && 
							 (it->second != String("565")) && 
							 (it->second != String("888")) && 
							 (it->second != String("555"))) throw E_invalid_parameter(String("f"));
					break;
				case 'm' : if ((it->second != String("true")) && (it->second != String("false")))
							throw E_invalid_parameter(String("m"));
					break;
				case 'n' :
					{
						std::istringstream iss(it->second.GetData());
						UInt32 value;

						if (!(iss >> value) || !iss.eof()) throw E_invalid_parameter(String("n"));

						break;
					}
				case 's' : // vérifier plus tard
					break;
				}
				find = True;
				break;
			}

		if (find == False) throw E_unknown_parameter(String(it->first));
	}

	// Recherche du fichier d'entrée

	std::map<Char, String>::iterator it = _parameters.find('i');
	if (it == _parameters.end()) throw E_input_file_undefined();

	it = _parameters.find('o');
	if (it == _parameters.end())
	{
		UInt32 ext = _parameters['i'].ReverseFind('.');

		if (ext != -1)
		{
			_parameters['o'] = _parameters['i'];
			_parameters['o'].Replace(String(".dds"), ext);
		}
		else _parameters['o'] = _parameters['i'] + String(".dds");
	}
}

Bool Defined(Char _key, std::map<Char, String> & _parameters)
{
	std::map<Char, String>::iterator it = _parameters.find(_key);

	if (it == _parameters.end()) return False;
	else return True;
}

