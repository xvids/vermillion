#include "client.h"

bool FirstFrame = false;
bool ClearDisconnect = false;

net_status_t g_nStatus;

void HookUserMessages()
{
	pUserMsgBase = (PUserMsg)offset.FindUserMsgBase();

	char reset_hud_msg[] = { 'R','e','s','e','t','H','U','D',0 };
	char set_fov_msg[] = { 'S','e','t','F','O','V',0 };
	char team_info_msg[] = { 'T','e','a','m','I','n','f','o',0 };
	char cur_weapon_msg[] = { 'C','u','r','W','e','a','p','o','n',0 };
	char death_msg_msg[] = { 'D','e','a','t','h','M','s','g',0 };
	char weaponlist_msg[] = { 'W','e','a','p','o','n','L','i','s','t',0 };
	char ammox_msg[] = { 'A','m','m','o','X',0 };

	pResetHUD = HookUserMsg( reset_hud_msg , ResetHUD );
	pSetFOV = HookUserMsg( set_fov_msg , SetFOV );
	pTeamInfo = HookUserMsg( team_info_msg , TeamInfo );
	pCurWeapon = HookUserMsg( cur_weapon_msg , CurWeapon );
	pDeathMsg = HookUserMsg( death_msg_msg , DeathMsg );
	pWeaponList = HookUserMsg( weaponlist_msg , WeaponList );
	pAmmoX = HookUserMsg( ammox_msg , AmmoX );
}

void HookEngineMessages()
{
	pEngineMsgBase = (PEngineMsg)offset.FindSVCMessages();

	char svc_sound_msg[] = { 's','v','c','_','s','o','u','n','d',0 };
	char svc_spawnstatic_sound_msg[] = { 's','v','c','_','s','p','a','w','n','s','t','a','t','i','c','s','o','u','n','d',0 };

	pSVC_sound = HookEngineMsg( svc_sound_msg , SVC_Sound );
	pSVC_SpawnStaticSound = HookEngineMsg( svc_spawnstatic_sound_msg , SVC_SpawnStaticSound );
}

void InitHack()
{
	g_Leis.InitHack();
}

void HUD_Frame( double time )
{
	if ( !FirstFrame )
	{
		g_Screen.iSize = sizeof( SCREENINFO );

		offset.HLType = g_Studio.IsHardware() + 1;

		HookUserMessages();
		HookEngineMessages();

		InitHack();
		
		FirstFrame = true;
	}
	
	g_Engine.pfnGetScreenInfo( &g_Screen );
	g_Engine.pNetAPI->Status( &( g_nStatus ) );

	g_Client.HUD_Frame( time );

	if ( g_nStatus.connected && !ClearDisconnect )
	{
		ClearDisconnect = true;
	}
	else if ( !g_nStatus.connected && ClearDisconnect || !g_Engine.pfnGetServerTime() )
	{
		g_Entity.ClearEntity();
		g_Sound.ClearSound();
		g_WeaponList.WeaponListClear();
		g_Players.ClearPlayers();

		ClearDisconnect = false;
	}
}

void HUD_Redraw( float time , int intermission )
{
	g_Client.HUD_Redraw( time , intermission );

	if ( g_nStatus.connected )
		g_Leis.HUD_Redraw();
}

int HUD_Key_Event( int down , int keynum , const char *pszCurrentBinding )
{
	if ( g_nStatus.connected )
	{
		if ( down )
		{
			g_Leis.HUD_Key_Event( keynum );

			if ( g_Menu.Visible || g_Menu.ExitMenu )
			{
				if ( keynum == cvar.key_NavEnter )
					return 0;
				else if ( keynum == cvar.key_TogglePanic )
					return 0;
				else if ( keynum == cvar.key_NavLeave )
					return 0;
				else if ( keynum == cvar.key_NavUp )
					return 0;
				else if ( keynum == cvar.key_NavDown )
					return 0;
				else if ( keynum == cvar.key_NavLeft )
					return 0;
				else if ( keynum == cvar.key_NavRight )
					return 0;
			}
		}
	}

	return g_Client.HUD_Key_Event( down , keynum , pszCurrentBinding );
}

void HUD_PlayerMove( struct playermove_s *ppmove , int server )
{
	g_Client.HUD_PlayerMove( ppmove , server );

	if ( g_nStatus.connected )
		g_Leis.HUD_PlayerMove( ppmove );
}

void V_CalcRefdef( struct ref_params_s *pparams )
{
	if ( g_nStatus.connected )
		g_Leis.V_CalcRefdef( pparams );
	else
		g_Client.V_CalcRefdef( pparams );
}

void StudioEntityLight( struct alight_s *plight )
{
	if ( g_nStatus.connected )
		g_Leis.StudioEntityLight( plight );

	g_Studio.StudioEntityLight( plight );
}

int HUD_AddEntity( int type , struct cl_entity_s *ent , const char *modelname )
{
	if ( g_nStatus.connected )
		return g_Leis.HUD_AddEntity( type , ent , modelname );
	else
		return g_Client.HUD_AddEntity( type , ent , modelname );
}

void CL_CreateMove( float frametime , usercmd_s *cmd , int active )
{
	g_Client.CL_CreateMove( frametime , cmd , active );

	if ( g_nStatus.connected )
	{
		g_Players.UpdatePlayerInfo();
		g_Leis.CL_CreateMove( frametime , cmd );
	}
}

void HUD_PostRunCmd( struct local_state_s *from , struct local_state_s *to , struct usercmd_s *cmd , int runfuncs , double time , unsigned int random_seed )
{
	g_Client.HUD_PostRunCmd( from , to , cmd , runfuncs , time , random_seed );

	if ( g_nStatus.connected )
		g_Leis.HUD_PostRunCmd( to , cmd , runfuncs , time , random_seed );
}

int HUD_UpdateClientData( client_data_t *pcldata , float flTime )
{
	if ( pcldata )
		g_WeaponList.WeaponListUpdate( pcldata->iWeaponBits );

	return g_Client.HUD_UpdateClientData( pcldata , flTime );
}

void HookFunction()
{
	g_pClient->HUD_Frame = HUD_Frame;
	g_pClient->HUD_Redraw = HUD_Redraw;
	g_pClient->HUD_Key_Event = HUD_Key_Event;
	g_pClient->HUD_PlayerMove = HUD_PlayerMove;
	g_pClient->V_CalcRefdef = V_CalcRefdef;
	g_pStudio->StudioEntityLight = StudioEntityLight;
	g_pClient->HUD_AddEntity = HUD_AddEntity;
	g_pClient->CL_CreateMove = CL_CreateMove;
	g_pClient->HUD_PostRunCmd = HUD_PostRunCmd;
	g_pClient->HUD_UpdateClientData = HUD_UpdateClientData;
}