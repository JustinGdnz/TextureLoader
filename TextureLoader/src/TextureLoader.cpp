#include <iostream>
#include <Windows.h>
#include "DsMap/DsMap.h"
#include "AppProcess/AppProcess.h"
#include <shellapi.h>
#include <filesystem>
#include "libzippp/libzippp.h"
#include <fstream>

// Variables utiles para la compatibilidad entre c++ y gms2
#define GM_EXPORT extern "C" __declspec(dllexport)
#define GM_TRUE (1.0)
#define GM_FALSE (0.0)

using namespace libzippp;
namespace filesystem = std::filesystem;

static bool tl_initialized = false;												// Sirve para comprobar si GMS2 cargo correctamente el DLL
static AppProcess GameProcess(L"SMM_WE.exe");									// Nuestro juego en una variable, para poder hacer el puente entre GMS2 y SMMWE

// Conseguir las variables de entorno (ej. "%appdata%")
char* GetEnv(const char* pPath)
{
	char* env;
	size_t len;
	errno_t err = _dupenv_s(&env, &len, pPath);

	return env;
}

// Inicializar el DLL por parte de GMS2
GM_EXPORT double dllinit()
{
	if (!tl_initialized)
		tl_initialized = true;
	return GM_TRUE;
}

// Exit, basicamente
GM_EXPORT double dllexit()
{
	if (tl_initialized)
		tl_initialized = false;
	return GM_FALSE;
}

// Registrar las funciones necesarias para los DsMaps
GM_EXPORT double RegisterCallbacks(char* arg1, char* arg2, char* arg3, char* arg4)
{
	DsMap::RegCallbacks(arg1, arg2, arg3, arg4);
	return GM_TRUE;
}

// Inicializar TL y cargar los procesos de SMM:WE
GM_EXPORT double tl_init()
{
	if (!tl_initialized) return GM_FALSE;

	GameProcess.AttachProcess();
	if (GameProcess.isValid()) return GM_TRUE;

	return GM_FALSE;
}

GM_EXPORT double tl_update()
{
	if (!tl_initialized) return GM_FALSE;
	return GM_TRUE;
}

// Inyectar la libreria libSMMWE en SMM:WE
GM_EXPORT double tl_injectdll()
{
	if (!tl_initialized) return GM_FALSE;

	if (GameProcess.isValid())
	{
		if (GameProcess.isRunning())
		{
			if (!GameProcess.hasModule("libSMMWE.dll"))
			{
				GameProcess.InjectDLL("libSMMWE.dll");
				return GM_TRUE;
			}
		}
	}

	return GM_FALSE;
}

// Comprobar si SMM:WE esta ejecutado
GM_EXPORT double tl_is_process_running()
{
	if (!tl_initialized) return GM_FALSE;
	if (!GameProcess.isValid()) return GM_FALSE;

	return GameProcess.isRunning();

}

// Comprobar si SMM:WE esta ejecutando el proceso para modificar los Sprites
GM_EXPORT double tl_has_module()
{
	if (!tl_initialized) return GM_FALSE;

	if (GameProcess.isValid())
	{
		if (GameProcess.isRunning())
		{
			return GameProcess.hasModule("libSMMWE.dll");
		}
	}

	return GM_FALSE;
}

// Devuelve la ruta de la variable de entorno
GM_EXPORT char* tl_get_environment_path(char* env)
{
	if (!tl_initialized) return (char*)"";
	return GetEnv(env);
}

// Abrir una carpeta en el explorador de archivos
GM_EXPORT double tl_open_folder(char* path)
{
	if (!tl_initialized) return GM_FALSE;

	filesystem::path oPath = path;
	ShellExecute(NULL, L"open", oPath.wstring().c_str(), NULL, NULL, SW_SHOWDEFAULT);

	return GM_TRUE;
}

// Abrir una url en el explorador web
GM_EXPORT double tl_open_url(char* url)
{
	if (!tl_initialized) return GM_FALSE;

	filesystem::path Path = url;

	// Mi canal de youtube pero hardcoded >:D
	ShellExecute(0, 0, Path.wstring().c_str(), 0, 0, SW_SHOW);

	return GM_TRUE;
}

// Crear los directorios necesarios para TL
GM_EXPORT double tl_create_directories()
{
	if (!tl_initialized) return GM_FALSE;
	filesystem::path LocalAppData = GetEnv("LOCALAPPDATA");
	filesystem::path TexturesPath = LocalAppData / "SMM_WE\\Textures\\Default";
	filesystem::path SpritesPath = LocalAppData / "SMM_WE\\Textures\\Default\\Sprites";
	filesystem::path BGPath = LocalAppData / "SMM_WE\\Textures\\Default\\Backgrounds";

	filesystem::create_directories(TexturesPath);
	filesystem::create_directory(SpritesPath);
	filesystem::create_directory(BGPath);

	return GM_TRUE;
}

GM_EXPORT double tl_zip_unzip_singlefile(char* zip_file, char* fname, char* output)
{
	if (!tl_initialized) return GM_FALSE;

	ZipArchive zip(zip_file);
	zip.open(ZipArchive::ReadOnly);

	ZipEntry single = zip.getEntry(fname);
	std::ofstream out(output, std::ios::binary);

	out.write((char*)single.readAsBinary(), single.getSize());

	out.close();
	zip.close();

	return GM_TRUE;
}