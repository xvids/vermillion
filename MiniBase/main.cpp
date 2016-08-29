#include "main.h"

#include <Wbemidl.h>

#pragma comment(lib, "wbemuuid.lib")

cl_clientfunc_t *g_pClient = nullptr;
cl_enginefunc_t *g_pEngine = nullptr;
engine_studio_api_t *g_pStudio = nullptr;

cl_clientfunc_t g_Client;
cl_enginefunc_t g_Engine;
engine_studio_api_t g_Studio;

PUserMsg pUserMsgBase;
PEngineMsg pEngineMsgBase;

SCREENINFO g_Screen;

BYTE bPreType = 0;
HANDLE hProcessReloadThread = 0;
PCHAR BaseDir;

DWORD WINAPI CheatEntry( LPVOID lpThreadParameter );

DWORD WINAPI ProcessReload( LPVOID lpThreadParameter )
{
	while ( true )
	{
		if ( FirstFrame )
		{
			offset.GetRenderType();

			if ( !offset.GetModuleInfo() )
				FirstFrame = false;
		}
		else
		{
			CreateThread( 0 , 0 , CheatEntry , 0 , 0 , 0 );
		}

		Sleep( 500 );
	}

	return 0;
}

DWORD WINAPI CheatEntry( LPVOID lpThreadParameter )
{
	if ( hProcessReloadThread )
	{
		TerminateThread( hProcessReloadThread , 0 );
		CloseHandle( hProcessReloadThread );
	}
	
	BYTE counter_find = 0;

start_hook:

	if ( counter_find == 100 )
	{
		char error_find[28] =
		{ 
			'F','i','n','d',' ',
			'C','l','/','E','n','/','S','t',' ',
			'M','o','d','u','l','e','s',' ',
			'E','r','r','o','r',0
		};

		offset.Error( error_find );
	}

	Sleep( 100 );
	counter_find++;

	if ( !offset.GetModuleInfo() )
	{
		goto start_hook;
	}

	DWORD ClientTable = offset.FindClientTable();

	if ( ClientTable )
	{
		g_pClient = (cl_clientfunc_t*)ClientTable;
		offset.CopyClient();

		if ( (DWORD)g_Client.Initialize )
		{
			DWORD EngineTable = offset.FindEngineTable();

			if ( EngineTable )
			{
				g_pEngine = (cl_enginefunc_t*)EngineTable;
				offset.CopyEngine();

				if ( (DWORD)g_Engine.V_CalcShake )
				{
					DWORD StudioTable = offset.FindStudioTable();

					if ( StudioTable )
					{
						g_pStudio = (engine_studio_api_t*)StudioTable;
						offset.CopyStudio();

						if ( (DWORD)g_Studio.StudioSetupSkin )
						{
							while ( !FirstFrame )
							{
								HookFunction();
								Sleep( 500 );
							}

							bPreType = offset.HLType;
							
							hProcessReloadThread = CreateThread( 0 , 0 , ProcessReload , 0 , 0 , 0 );
						}
						else
							goto start_hook;
					}
					else
					{
						goto start_hook;
					}
				}
				else
					goto start_hook;
			}
			else
			{
				goto start_hook;
			}
		}
		else
			goto start_hook;
	}
	else
	{
		goto start_hook;
	}

	return 0;
}

char* GetHDD()
{
	char pcid_root_cimv2[] = { 'R','O','O','T','\\\\','C','I','M','V','2',0 };
	char pcid_wql[] = { 'W','Q','L',0 };
	char pcid_w32[] =
	{
		'S','E','L','E','C','T',' ','*',' ','F','R','O','M',' ',
		'W','i','n','3','2','_','P','h','y','s','i','c','a','l',
		'M','e','d','i','a',0
	};
	char pcid_prc[] = { 'S','e','r','i','a','l','N','u','m','b','e','r',0 };

	HRESULT hres;

	char* Result = NULL;

	IWbemLocator *pLoc = NULL;
	IWbemServices *pSvc = NULL;
	IEnumWbemClassObject* pEnumerator = NULL;
	IWbemClassObject *pclsObj = NULL;
	ULONG uReturn = 0;
	VARIANT vtProp;

	hres = CoInitializeEx( 0 , COINIT_MULTITHREADED );
	hres = CoInitializeSecurity( NULL , -1 , NULL , NULL , RPC_C_AUTHN_LEVEL_DEFAULT ,
								 RPC_C_IMP_LEVEL_IMPERSONATE , NULL , EOAC_NONE , NULL );

	if ( FAILED( hres ) )
	{
		CoUninitialize();
	}

	hres = CoCreateInstance( CLSID_WbemLocator , 0 , CLSCTX_INPROC_SERVER , IID_IWbemLocator , (LPVOID *)&pLoc );

	if ( FAILED( hres ) )
	{
		CoUninitialize();
	}

	hres = pLoc->ConnectServer( ConvertStringToBSTR( pcid_root_cimv2 ) , NULL , NULL , 0 , NULL , 0 , 0 , &pSvc );

	if ( FAILED( hres ) )
	{
		pLoc->Release();
		CoUninitialize();
	}

	hres = CoSetProxyBlanket( pSvc , RPC_C_AUTHN_WINNT , RPC_C_AUTHZ_NONE , NULL , RPC_C_AUTHN_LEVEL_CALL ,
							  RPC_C_IMP_LEVEL_IMPERSONATE , NULL , EOAC_NONE );

	if ( FAILED( hres ) )
	{
		pSvc->Release();
		pLoc->Release();
		CoUninitialize();
	}

	hres = pSvc->ExecQuery( ConvertStringToBSTR( pcid_wql ) ,
							ConvertStringToBSTR( pcid_w32 ) ,
							WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY , NULL , &pEnumerator );

	if ( FAILED( hres ) )
	{
		pSvc->Release();
		pLoc->Release();
		CoUninitialize();
	}

	HRESULT hr = pEnumerator->Next( WBEM_INFINITE , 1 , &pclsObj , &uReturn );
	hr = pclsObj->Get( ConvertStringToBSTR( pcid_prc ) , 0 , &vtProp , 0 , 0 );
	Result = ConvertBSTRToString( vtProp.bstrVal );
	VariantClear( &vtProp );

	pclsObj->Release();

	pSvc->Release();
	pLoc->Release();
	pEnumerator->Release();
	CoUninitialize();

	return Result;
}

#if (INJT_TYPE == 1)
	BOOL WINAPI DllMain( PCHAR DllPath , PCHAR HDD , DWORD fdwReason , LPVOID lpReserved )
#else
	BOOL WINAPI DllMain( HINSTANCE hinstDLL , DWORD fdwReason , LPVOID lpReserved )
#endif
{
	switch ( fdwReason )
	{
	case DLL_PROCESS_ATTACH:

#if (INJT_TYPE == 1)
		if ( *(PWORD)DllPath == IMAGE_DOS_SIGNATURE || ( native_strcmp( HDD , GetHDD() ) != 0 ) )
			return TRUE;
#endif
		BaseDir = (PCHAR)HeapAlloc( GetProcessHeap() , HEAP_ZERO_MEMORY , MAX_PATH );

#if (INJT_TYPE == 0)
		GetModuleFileNameA( hinstDLL , BaseDir , MAX_PATH );
#endif

#if (INJT_TYPE == 1)	
		native_strcpy( BaseDir , DllPath );
#endif
		CreateThread( 0 , 0 , CheatEntry , 0 , 0 , 0 );

		return TRUE;
	}

	return FALSE;
}