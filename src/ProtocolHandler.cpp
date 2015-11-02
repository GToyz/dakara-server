/******************************************************************************
    Copyright (C) 2002-2015 Argentum Online & Dakara Online Developers

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU Affero General Public License as
    published by the Free Software Foundation, either version 3 of the
    License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Affero General Public License for more details.

    You should have received a copy of the GNU Affero General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
******************************************************************************/

#include "ProtocolHandler.h"
#include "allheaders.h"
#include "enums.h"
#include "vb6compat.h"
#include "Crypto.h"

void HandleIncomingData(int UserIndex) {

	int k = 0;
	int lastvalidpos = UserList[UserIndex].sockctx->incomingData->getReadPos();

	UserList[UserIndex].sockctx->incomingDataAvailable = false;

	while (UserList[UserIndex].sockctx->incomingData->length() > 0 && k < MAX_PACKETS_PER_ITER) {
		// int startpos = UserList[UserIndex].sockctx->incomingData->getReadPos();
		try {
			HandleIncomingDataOnePacket(UserIndex);
			++k;
			if (!UserList[UserIndex].ConnIDValida) {
				break;
			}
			lastvalidpos = UserList[UserIndex].sockctx->incomingData->getReadPos();
		} catch (bytequeue_data_error& ex) {
			std::cerr << "bytequeue_data_error: " << ex.what() << std::endl;
			CloseSocket(UserIndex);
			break;
		} catch (insufficient_data_error& ex) {
			// Ignored.
			UserList[UserIndex].sockctx->incomingData->commitRead(lastvalidpos);
			lastvalidpos = 0;
			break;
		}
	}

	if (UserList[UserIndex].ConnIDValida) {
		UserList[UserIndex].sockctx->ConnIgnoreIncomingData = (UserList[UserIndex].sockctx->incomingData->length() > 0 && k == MAX_PACKETS_PER_ITER);

		if (lastvalidpos > 0) {
			UserList[UserIndex].sockctx->incomingData->commitRead(lastvalidpos);
		}

		FlushBuffer(UserIndex);
	}
}

void HandleIncomingDataOnePacket(int UserIndex) {
	/* '*************************************************** */
	/* 'Author: Juan Martín Sotuyo Dodero (Maraxus) */
	/* 'Last Modification: 01/09/07 */
	/* ' */
	/* '*************************************************** */
	int packetID;

	packetID = UserList[UserIndex].sockctx->incomingData->PeekByte();

	/* 'Does the packet requires a logged user?? */
	if (!(packetID == ClientPacketID_ThrowDices || packetID == ClientPacketID_LoginExistingChar
			|| packetID == ClientPacketID_LoginNewChar)) {

		/* 'Is the user actually logged? */
		if (!UserList[UserIndex].flags.UserLogged) {
			CloseSocket(UserIndex);
			return;

			/* 'He is logged. Reset idle counter if id is valid. */
		} else if (packetID <= LAST_CLIENT_PACKET_ID) {
			UserList[UserIndex].Counters.IdleCount = 0;
		}
	} else if (packetID <= LAST_CLIENT_PACKET_ID) {
		UserList[UserIndex].Counters.IdleCount = 0;

		/* 'Is the user logged? */
		if (UserList[UserIndex].flags.UserLogged) {
			CloseSocket(UserIndex);
			return;
		}
	}

	/* ' Ante cualquier paquete, pierde la proteccion de ser atacado. */
	UserList[UserIndex].flags.NoPuedeSerAtacado = false;

	switch (packetID) {
	/* 'OLOGIN */
	case ClientPacketID_LoginExistingChar:
		HandleLoginExistingChar(UserIndex);

		/* 'TIRDAD */
		break;

	case ClientPacketID_ThrowDices:
		HandleThrowDices(UserIndex);

		/* 'NLOGIN */
		break;

	case ClientPacketID_LoginNewChar:
		HandleLoginNewChar(UserIndex);

		/* '; */
		break;

	case ClientPacketID_Talk:
		HandleTalk(UserIndex);

		/* '- */
		break;

	case ClientPacketID_Yell:
		HandleYell(UserIndex);

		/* '\ */
		break;

	case ClientPacketID_Whisper:
		HandleWhisper(UserIndex);

		/* 'M */
		break;

	case ClientPacketID_Walk:
		HandleWalk(UserIndex);

		/* 'RPU */
		break;

	case ClientPacketID_RequestPositionUpdate:
		HandleRequestPositionUpdate(UserIndex);

		/* 'AT */
		break;

	case ClientPacketID_Attack:
		HandleAttack(UserIndex);

		/* 'AG */
		break;

	case ClientPacketID_PickUp:
		HandlePickUp(UserIndex);

		/* '/SEG & SEG  (SEG's behaviour has to be coded in the client) */
		break;

	case ClientPacketID_SafeToggle:
		HandleSafeToggle(UserIndex);

		break;

	case ClientPacketID_ResuscitationSafeToggle:
		HandleResuscitationToggle(UserIndex);

		/* 'GLINFO */
		break;

	case ClientPacketID_RequestGuildLeaderInfo:
		HandleRequestGuildLeaderInfo(UserIndex);

		/* 'ATR */
		break;

	case ClientPacketID_RequestAtributes:
		HandleRequestAtributes(UserIndex);

		/* 'FAMA */
		break;

	case ClientPacketID_RequestFame:
		HandleRequestFame(UserIndex);

		/* 'ESKI */
		break;

	case ClientPacketID_RequestSkills:
		HandleRequestSkills(UserIndex);

		/* 'FEST */
		break;

	case ClientPacketID_RequestMiniStats:
		HandleRequestMiniStats(UserIndex);

		/* 'FINCOM */
		break;

	case ClientPacketID_CommerceEnd:
		HandleCommerceEnd(UserIndex);

		break;

	case ClientPacketID_CommerceChat:
		HandleCommerceChat(UserIndex);

		/* 'FINCOMUSU */
		break;

	case ClientPacketID_UserCommerceEnd:
		HandleUserCommerceEnd(UserIndex);

		break;

	case ClientPacketID_UserCommerceConfirm:
		HandleUserCommerceConfirm(UserIndex);

		/* 'FINBAN */
		break;

	case ClientPacketID_BankEnd:
		HandleBankEnd(UserIndex);

		/* 'COMUSUOK */
		break;

	case ClientPacketID_UserCommerceOk:
		HandleUserCommerceOk(UserIndex);

		/* 'COMUSUNO */
		break;

	case ClientPacketID_UserCommerceReject:
		HandleUserCommerceReject(UserIndex);

		/* 'TI */
		break;

	case ClientPacketID_Drop:
		HandleDrop(UserIndex);

		/* 'LH */
		break;

	case ClientPacketID_CastSpell:
		HandleCastSpell(UserIndex);

		/* 'LC */
		break;

	case ClientPacketID_LeftClick:
		HandleLeftClick(UserIndex);

		/* 'RC */
		break;

	case ClientPacketID_DoubleClick:
		HandleDoubleClick(UserIndex);

		/* 'UK */
		break;

	case ClientPacketID_Work:
		HandleWork(UserIndex);

		/* 'UMH */
		break;

	case ClientPacketID_UseSpellMacro:
		HandleUseSpellMacro(UserIndex);

		/* 'USA */
		break;

	case ClientPacketID_UseItem:
		HandleUseItem(UserIndex);

		/* 'CNS */
		break;

	case ClientPacketID_CraftBlacksmith:
		HandleCraftBlacksmith(UserIndex);

		/* 'CNC */
		break;

	case ClientPacketID_CraftCarpenter:
		HandleCraftCarpenter(UserIndex);

		/* 'WLC */
		break;

	case ClientPacketID_WorkLeftClick:
		HandleWorkLeftClick(UserIndex);

		/* 'CIG */
		break;

	case ClientPacketID_CreateNewGuild:
		HandleCreateNewGuild(UserIndex);

		/* 'INFS */
		break;

	case ClientPacketID_SpellInfo:
		HandleSpellInfo(UserIndex);

		/* 'EQUI */
		break;

	case ClientPacketID_EquipItem:
		HandleEquipItem(UserIndex);

		/* 'CHEA */
		break;

	case ClientPacketID_ChangeHeading:
		HandleChangeHeading(UserIndex);

		/* 'SKSE */
		break;

	case ClientPacketID_ModifySkills:
		HandleModifySkills(UserIndex);

		/* 'ENTR */
		break;

	case ClientPacketID_Train:
		HandleTrain(UserIndex);

		/* 'COMP */
		break;

	case ClientPacketID_CommerceBuy:
		HandleCommerceBuy(UserIndex);

		/* 'RETI */
		break;

	case ClientPacketID_BankExtractItem:
		HandleBankExtractItem(UserIndex);

		/* 'VEND */
		break;

	case ClientPacketID_CommerceSell:
		HandleCommerceSell(UserIndex);

		/* 'DEPO */
		break;

	case ClientPacketID_BankDeposit:
		HandleBankDeposit(UserIndex);

		/* 'DEMSG */
		break;

	case ClientPacketID_ForumPost:
		HandleForumPost(UserIndex);

		/* 'DESPHE */
		break;

	case ClientPacketID_MoveSpell:
		HandleMoveSpell(UserIndex);

		break;

	case ClientPacketID_MoveBank:
		HandleMoveBank(UserIndex);

		/* 'DESCOD */
		break;

	case ClientPacketID_ClanCodexUpdate:
		HandleClanCodexUpdate(UserIndex);

		/* 'OFRECER */
		break;

	case ClientPacketID_UserCommerceOffer:
		HandleUserCommerceOffer(UserIndex);

		/* 'ACEPPEAT */
		break;

	case ClientPacketID_GuildAcceptPeace:
		HandleGuildAcceptPeace(UserIndex);

		/* 'RECPALIA */
		break;

	case ClientPacketID_GuildRejectAlliance:
		HandleGuildRejectAlliance(UserIndex);

		/* 'RECPPEAT */
		break;

	case ClientPacketID_GuildRejectPeace:
		HandleGuildRejectPeace(UserIndex);

		/* 'ACEPALIA */
		break;

	case ClientPacketID_GuildAcceptAlliance:
		HandleGuildAcceptAlliance(UserIndex);

		/* 'PEACEOFF */
		break;

	case ClientPacketID_GuildOfferPeace:
		HandleGuildOfferPeace(UserIndex);

		/* 'ALLIEOFF */
		break;

	case ClientPacketID_GuildOfferAlliance:
		HandleGuildOfferAlliance(UserIndex);

		/* 'ALLIEDET */
		break;

	case ClientPacketID_GuildAllianceDetails:
		HandleGuildAllianceDetails(UserIndex);

		/* 'PEACEDET */
		break;

	case ClientPacketID_GuildPeaceDetails:
		HandleGuildPeaceDetails(UserIndex);

		/* 'ENVCOMEN */
		break;

	case ClientPacketID_GuildRequestJoinerInfo:
		HandleGuildRequestJoinerInfo(UserIndex);

		/* 'ENVALPRO */
		break;

	case ClientPacketID_GuildAlliancePropList:
		HandleGuildAlliancePropList(UserIndex);

		/* 'ENVPROPP */
		break;

	case ClientPacketID_GuildPeacePropList:
		HandleGuildPeacePropList(UserIndex);

		/* 'DECGUERR */
		break;

	case ClientPacketID_GuildDeclareWar:
		HandleGuildDeclareWar(UserIndex);

		/* 'NEWWEBSI */
		break;

	case ClientPacketID_GuildNewWebsite:
		HandleGuildNewWebsite(UserIndex);

		/* 'ACEPTARI */
		break;

	case ClientPacketID_GuildAcceptNewMember:
		HandleGuildAcceptNewMember(UserIndex);

		/* 'RECHAZAR */
		break;

	case ClientPacketID_GuildRejectNewMember:
		HandleGuildRejectNewMember(UserIndex);

		/* 'ECHARCLA */
		break;

	case ClientPacketID_GuildKickMember:
		HandleGuildKickMember(UserIndex);

		/* 'ACTGNEWS */
		break;

	case ClientPacketID_GuildUpdateNews:
		HandleGuildUpdateNews(UserIndex);

		/* '1HRINFO< */
		break;

	case ClientPacketID_GuildMemberInfo:
		HandleGuildMemberInfo(UserIndex);

		/* 'ABREELEC */
		break;

	case ClientPacketID_GuildOpenElections:
		HandleGuildOpenElections(UserIndex);

		/* 'SOLICITUD */
		break;

	case ClientPacketID_GuildRequestMembership:
		HandleGuildRequestMembership(UserIndex);

		/* 'CLANDETAILS */
		break;

	case ClientPacketID_GuildRequestDetails:
		HandleGuildRequestDetails(UserIndex);

		/* '/ONLINE */
		break;

	case ClientPacketID_Online:
		HandleOnline(UserIndex);

		/* '/SALIR */
		break;

	case ClientPacketID_Quit:
		HandleQuit(UserIndex);

		/* '/SALIRCLAN */
		break;

	case ClientPacketID_GuildLeave:
		HandleGuildLeave(UserIndex);

		/* '/BALANCE */
		break;

	case ClientPacketID_RequestAccountState:
		HandleRequestAccountState(UserIndex);

		/* '/QUIETO */
		break;

	case ClientPacketID_PetStand:
		HandlePetStand(UserIndex);

		/* '/ACOMPANAR */
		break;

	case ClientPacketID_PetFollow:
		HandlePetFollow(UserIndex);

		/* '/LIBERAR */
		break;

	case ClientPacketID_ReleasePet:
		HandleReleasePet(UserIndex);

		/* '/ENTRENAR */
		break;

	case ClientPacketID_TrainList:
		HandleTrainList(UserIndex);

		/* '/DESCANSAR */
		break;

	case ClientPacketID_Rest:
		HandleRest(UserIndex);

		/* '/MEDITAR */
		break;

	case ClientPacketID_Meditate:
		HandleMeditate(UserIndex);

		/* '/RESUCITAR */
		break;

	case ClientPacketID_Resucitate:
		HandleResucitate(UserIndex);

		/* '/CURAR */
		break;

	case ClientPacketID_Heal:
		HandleHeal(UserIndex);

		/* '/AYUDA */
		break;

	case ClientPacketID_Help:
		HandleHelp(UserIndex);

		/* '/EST */
		break;

	case ClientPacketID_RequestStats:
		HandleRequestStats(UserIndex);

		/* '/COMERCIAR */
		break;

	case ClientPacketID_CommerceStart:
		HandleCommerceStart(UserIndex);

		/* '/BOVEDA */
		break;

	case ClientPacketID_BankStart:
		HandleBankStart(UserIndex);

		/* '/ENLISTAR */
		break;

	case ClientPacketID_Enlist:
		HandleEnlist(UserIndex);

		/* '/INFORMACION */
		break;

	case ClientPacketID_Information:
		HandleInformation(UserIndex);

		/* '/RECOMPENSA */
		break;

	case ClientPacketID_Reward:
		HandleReward(UserIndex);

		/* '/MOTD */
		break;

	case ClientPacketID_RequestMOTD:
		HandleRequestMOTD(UserIndex);

		/* '/UPTIME */
		break;

	case ClientPacketID_UpTime:
		HandleUpTime(UserIndex);

		/* '/SALIRPARTY */
		break;

	case ClientPacketID_PartyLeave:
		HandlePartyLeave(UserIndex);

		/* '/CREARPARTY */
		break;

	case ClientPacketID_PartyCreate:
		HandlePartyCreate(UserIndex);

		/* '/PARTY */
		break;

	case ClientPacketID_PartyJoin:
		HandlePartyJoin(UserIndex);

		/* '/ENCUESTA ( with no params ) */
		break;

	case ClientPacketID_Inquiry:
		HandleInquiry(UserIndex);

		/* '/CMSG */
		break;

	case ClientPacketID_GuildMessage:
		HandleGuildMessage(UserIndex);

		/* '/PMSG */
		break;

	case ClientPacketID_PartyMessage:
		HandlePartyMessage(UserIndex);

		/* '/CENTINELA */
		break;

	case ClientPacketID_CentinelReport:
		HandleCentinelReport(UserIndex);

		/* '/ONLINECLAN */
		break;

	case ClientPacketID_GuildOnline:
		HandleGuildOnline(UserIndex);

		/* '/ONLINEPARTY */
		break;

	case ClientPacketID_PartyOnline:
		HandlePartyOnline(UserIndex);

		/* '/BMSG */
		break;

	case ClientPacketID_CouncilMessage:
		HandleCouncilMessage(UserIndex);

		/* '/ROL */
		break;

	case ClientPacketID_RoleMasterRequest:
		HandleRoleMasterRequest(UserIndex);

		/* '/GM */
		break;

	case ClientPacketID_GMRequest:
		HandleGMRequest(UserIndex);

		/* '/_BUG */
		break;

	case ClientPacketID_bugReport:
		HandleBugReport(UserIndex);

		/* '/DESC */
		break;

	case ClientPacketID_ChangeDescription:
		HandleChangeDescription(UserIndex);

		/* '/VOTO */
		break;

	case ClientPacketID_GuildVote:
		HandleGuildVote(UserIndex);

		/* '/PENAS */
		break;

	case ClientPacketID_Punishments:
		HandlePunishments(UserIndex);

		/* '/CONTRASENA */
		break;

	case ClientPacketID_ChangePassword:
		HandleChangePassword(UserIndex);

		/* '/APOSTAR */
		break;

	case ClientPacketID_Gamble:
		HandleGamble(UserIndex);

		/* '/ENCUESTA ( with parameters ) */
		break;

	case ClientPacketID_InquiryVote:
		HandleInquiryVote(UserIndex);

		/* '/RETIRAR ( with no arguments ) */
		break;

	case ClientPacketID_LeaveFaction:
		HandleLeaveFaction(UserIndex);

		/* '/RETIRAR ( with arguments ) */
		break;

	case ClientPacketID_BankExtractGold:
		HandleBankExtractGold(UserIndex);

		/* '/DEPOSITAR */
		break;

	case ClientPacketID_BankDepositGold:
		HandleBankDepositGold(UserIndex);

		/* '/DENUNCIAR */
		break;

	case ClientPacketID_Denounce:
		HandleDenounce(UserIndex);

		/* '/FUNDARCLAN */
		break;

	case ClientPacketID_GuildFundate:
		HandleGuildFundate(UserIndex);

		break;

	case ClientPacketID_GuildFundation:
		HandleGuildFundation(UserIndex);

		/* '/ECHARPARTY */
		break;

	case ClientPacketID_PartyKick:
		HandlePartyKick(UserIndex);

		/* '/PARTYLIDER */
		break;

	case ClientPacketID_PartySetLeader:
		HandlePartySetLeader(UserIndex);

		/* '/ACCEPTPARTY */
		break;

	case ClientPacketID_PartyAcceptMember:
		HandlePartyAcceptMember(UserIndex);

		/* '/PING */
		break;

	case ClientPacketID_Ping:
		HandlePing(UserIndex);

		break;

	case ClientPacketID_RequestPartyForm:
		HandlePartyForm(UserIndex);

		break;

	case ClientPacketID_ItemUpgrade:
		HandleItemUpgrade(UserIndex);

		/* 'GM Messages */
		break;

	case ClientPacketID_GMCommands:
		HandleGMCommands(UserIndex);

		break;

	case ClientPacketID_InitCrafting:
		HandleInitCrafting(UserIndex);

		break;

	case ClientPacketID_Home:
		HandleHome(UserIndex);

		break;

	case ClientPacketID_ShowGuildNews:
		HandleShowGuildNews(UserIndex);

		break;

	case ClientPacketID_ShareNpc:
		HandleShareNpc(UserIndex);

		break;

	case ClientPacketID_StopSharingNpc:
		HandleStopSharingNpc(UserIndex);

		break;

	case ClientPacketID_Consultation:
		HandleConsultation(UserIndex);

		break;

	case ClientPacketID_moveItem:
		HandleMoveItem(UserIndex);

		/* # IF SeguridadAlkon THEN */
		/* # ELSE */
		break;

	default:
		/* 'ERROR : Abort! */
		CerrarUserIndex(UserIndex);
		/* # END IF */
		break;
	}
}

void HandleGMCommands(int UserIndex) {
	/* '*************************************************** */
	/* 'Author: Unknown */
	/* 'Last Modification: - */
	/* ' */
	/* '*************************************************** */

	int Command;

	UserList[UserIndex].sockctx->incomingData->ReadByte();

	Command = UserList[UserIndex].sockctx->incomingData->PeekByte();

	switch (Command) {
	/* '/GMSG */
	case eGMCommands_GMMessage:
		HandleGMMessage(UserIndex);

		/* '/SHOWNAME */
		break;

	case eGMCommands_showName:
		HandleShowName(UserIndex);

		break;

	case eGMCommands_OnlineRoyalArmy:
		HandleOnlineRoyalArmy(UserIndex);

		/* '/ONLINECAOS */
		break;

	case eGMCommands_OnlineChaosLegion:
		HandleOnlineChaosLegion(UserIndex);

		/* '/IRCERCA */
		break;

	case eGMCommands_GoNearby:
		HandleGoNearby(UserIndex);

		/* '/REM */
		break;

	case eGMCommands_comment:
		HandleComment(UserIndex);

		/* '/HORA */
		break;

	case eGMCommands_serverTime:
		HandleServerTime(UserIndex);

		/* '/DONDE */
		break;

	case eGMCommands_Where:
		HandleWhere(UserIndex);

		/* '/NENE */
		break;

	case eGMCommands_CreaturesInMap:
		HandleCreaturesInMap(UserIndex);

		/* '/TELEPLOC */
		break;

	case eGMCommands_WarpMeToTarget:
		HandleWarpMeToTarget(UserIndex);

		/* '/TELEP */
		break;

	case eGMCommands_WarpChar:
		HandleWarpChar(UserIndex);

		/* '/SILENCIAR */
		break;

	case eGMCommands_Silence:
		HandleSilence(UserIndex);

		/* '/SHOW SOS */
		break;

	case eGMCommands_SOSShowList:
		HandleSOSShowList(UserIndex);

		/* 'SOSDONE */
		break;

	case eGMCommands_SOSRemove:
		HandleSOSRemove(UserIndex);

		/* '/IRA */
		break;

	case eGMCommands_GoToChar:
		HandleGoToChar(UserIndex);

		/* '/INVISIBLE */
		break;

	case eGMCommands_invisible:
		HandleInvisible(UserIndex);

		/* '/PANELGM */
		break;

	case eGMCommands_GMPanel:
		HandleGMPanel(UserIndex);

		/* 'LISTUSU */
		break;

	case eGMCommands_RequestUserList:
		HandleRequestUserList(UserIndex);

		/* '/TRABAJANDO */
		break;

	case eGMCommands_Working:
		HandleWorking(UserIndex);

		/* '/OCULTANDO */
		break;

	case eGMCommands_Hiding:
		HandleHiding(UserIndex);

		/* '/CARCEL */
		break;

	case eGMCommands_Jail:
		HandleJail(UserIndex);

		/* '/RMATA */
		break;

	case eGMCommands_KillNPC:
		HandleKillNPC(UserIndex);

		/* '/ADVERTENCIA */
		break;

	case eGMCommands_WarnUser:
		HandleWarnUser(UserIndex);

		/* '/MOD */
		break;

	case eGMCommands_EditChar:
		HandleEditChar(UserIndex);

		/* '/INFO */
		break;

	case eGMCommands_RequestCharInfo:
		HandleRequestCharInfo(UserIndex);

		/* '/STAT */
		break;

	case eGMCommands_RequestCharStats:
		HandleRequestCharStats(UserIndex);

		/* '/BAL */
		break;

	case eGMCommands_RequestCharGold:
		HandleRequestCharGold(UserIndex);

		/* '/INV */
		break;

	case eGMCommands_RequestCharInventory:
		HandleRequestCharInventory(UserIndex);

		/* '/BOV */
		break;

	case eGMCommands_RequestCharBank:
		HandleRequestCharBank(UserIndex);

		/* '/SKILLS */
		break;

	case eGMCommands_RequestCharSkills:
		HandleRequestCharSkills(UserIndex);

		/* '/REVIVIR */
		break;

	case eGMCommands_ReviveChar:
		HandleReviveChar(UserIndex);

		/* '/ONLINEGM */
		break;

	case eGMCommands_OnlineGM:
		HandleOnlineGM(UserIndex);

		/* '/ONLINEMAP */
		break;

	case eGMCommands_OnlineMap:
		HandleOnlineMap(UserIndex);

		/* '/PERDON */
		break;

	case eGMCommands_Forgive:
		HandleForgive(UserIndex);

		/* '/ECHAR */
		break;

	case eGMCommands_Kick:
		HandleKick(UserIndex);

		/* '/EJECUTAR */
		break;

	case eGMCommands_Execute:
		HandleExecute(UserIndex);

		/* '/BAN */
		break;

	case eGMCommands_BanChar:
		HandleBanChar(UserIndex);

		/* '/UNBAN */
		break;

	case eGMCommands_UnbanChar:
		HandleUnbanChar(UserIndex);

		/* '/SEGUIR */
		break;

	case eGMCommands_NPCFollow:
		HandleNPCFollow(UserIndex);

		/* '/SUM */
		break;

	case eGMCommands_SummonChar:
		HandleSummonChar(UserIndex);

		/* '/CC */
		break;

	case eGMCommands_SpawnListRequest:
		HandleSpawnListRequest(UserIndex);

		/* 'SPA */
		break;

	case eGMCommands_SpawnCreature:
		HandleSpawnCreature(UserIndex);

		/* '/RESETINV */
		break;

	case eGMCommands_ResetNPCInventory:
		HandleResetNPCInventory(UserIndex);

		/* '/LIMPIAR */
		break;

	case eGMCommands_CleanWorld:
		HandleCleanWorld(UserIndex);

		/* '/RMSG */
		break;

	case eGMCommands_ServerMessage:
		HandleServerMessage(UserIndex);

		/* '/MAPMSG */
		break;

	case eGMCommands_MapMessage:
		HandleMapMessage(UserIndex);

		/* '/NICK2IP */
		break;

	case eGMCommands_NickToIP:
		HandleNickToIP(UserIndex);

		/* '/IP2NICK */
		break;

	case eGMCommands_IPToNick:
		HandleIPToNick(UserIndex);

		/* '/ONCLAN */
		break;

	case eGMCommands_GuildOnlineMembers:
		HandleGuildOnlineMembers(UserIndex);

		/* '/CT */
		break;

	case eGMCommands_TeleportCreate:
		HandleTeleportCreate(UserIndex);

		/* '/DT */
		break;

	case eGMCommands_TeleportDestroy:
		HandleTeleportDestroy(UserIndex);

		/* '/LLUVIA */
		break;

	case eGMCommands_RainToggle:
		HandleRainToggle(UserIndex);

		/* '/SETDESC */
		break;

	case eGMCommands_SetCharDescription:
		HandleSetCharDescription(UserIndex);

		/* '/FORCEMIDIMAP */
		break;

	case eGMCommands_ForceMIDIToMap:
		HanldeForceMIDIToMap(UserIndex);

		/* '/FORCEWAVMAP */
		break;

	case eGMCommands_ForceWAVEToMap:
		HandleForceWAVEToMap(UserIndex);

		/* '/REALMSG */
		break;

	case eGMCommands_RoyalArmyMessage:
		HandleRoyalArmyMessage(UserIndex);

		/* '/CAOSMSG */
		break;

	case eGMCommands_ChaosLegionMessage:
		HandleChaosLegionMessage(UserIndex);

		/* '/CIUMSG */
		break;

	case eGMCommands_CitizenMessage:
		HandleCitizenMessage(UserIndex);

		/* '/CRIMSG */
		break;

	case eGMCommands_CriminalMessage:
		HandleCriminalMessage(UserIndex);

		/* '/TALKAS */
		break;

	case eGMCommands_TalkAsNPC:
		HandleTalkAsNPC(UserIndex);

		/* '/MASSDEST */
		break;

	case eGMCommands_DestroyAllItemsInArea:
		HandleDestroyAllItemsInArea(UserIndex);

		/* '/ACEPTCONSE */
		break;

	case eGMCommands_AcceptRoyalCouncilMember:
		HandleAcceptRoyalCouncilMember(UserIndex);

		/* '/ACEPTCONSECAOS */
		break;

	case eGMCommands_AcceptChaosCouncilMember:
		HandleAcceptChaosCouncilMember(UserIndex);

		/* '/PISO */
		break;

	case eGMCommands_ItemsInTheFloor:
		HandleItemsInTheFloor(UserIndex);

		/* '/ESTUPIDO */
		break;

	case eGMCommands_MakeDumb:
		HandleMakeDumb(UserIndex);

		/* '/NOESTUPIDO */
		break;

	case eGMCommands_MakeDumbNoMore:
		HandleMakeDumbNoMore(UserIndex);

		/* '/DUMPSECURITY */
		break;

	case eGMCommands_DumpIPTables:
		HandleDumpIPTables(UserIndex);

		/* '/KICKCONSE */
		break;

	case eGMCommands_CouncilKick:
		HandleCouncilKick(UserIndex);

		/* '/TRIGGER */
		break;

	case eGMCommands_SetTrigger:
		HandleSetTrigger(UserIndex);

		/* '/TRIGGER with no args */
		break;

	case eGMCommands_AskTrigger:
		HandleAskTrigger(UserIndex);

		/* '/BANIPLIST */
		break;

	case eGMCommands_BannedIPList:
		HandleBannedIPList(UserIndex);

		/* '/BANIPRELOAD */
		break;

	case eGMCommands_BannedIPReload:
		HandleBannedIPReload(UserIndex);

		/* '/MIEMBROSCLAN */
		break;

	case eGMCommands_GuildMemberList:
		HandleGuildMemberList(UserIndex);

		/* '/BANCLAN */
		break;

	case eGMCommands_GuildBan:
		HandleGuildBan(UserIndex);

		/* '/BANIP */
		break;

	case eGMCommands_BanIP:
		HandleBanIP(UserIndex);

		/* '/UNBANIP */
		break;

	case eGMCommands_UnbanIP:
		HandleUnbanIP(UserIndex);

		/* '/CI */
		break;

	case eGMCommands_CreateItem:
		HandleCreateItem(UserIndex);

		/* '/DEST */
		break;

	case eGMCommands_DestroyItems:
		HandleDestroyItems(UserIndex);

		/* '/NOCAOS */
		break;

	case eGMCommands_ChaosLegionKick:
		HandleChaosLegionKick(UserIndex);

		/* '/NOREAL */
		break;

	case eGMCommands_RoyalArmyKick:
		HandleRoyalArmyKick(UserIndex);

		/* '/FORCEMIDI */
		break;

	case eGMCommands_ForceMIDIAll:
		HandleForceMIDIAll(UserIndex);

		/* '/FORCEWAV */
		break;

	case eGMCommands_ForceWAVEAll:
		HandleForceWAVEAll(UserIndex);

		/* '/BORRARPENA */
		break;

	case eGMCommands_RemovePunishment:
		HandleRemovePunishment(UserIndex);

		/* '/BLOQ */
		break;

	case eGMCommands_TileBlockedToggle:
		HandleTileBlockedToggle(UserIndex);

		/* '/MATA */
		break;

	case eGMCommands_KillNPCNoRespawn:
		HandleKillNPCNoRespawn(UserIndex);

		/* '/MASSKILL */
		break;

	case eGMCommands_KillAllNearbyNPCs:
		HandleKillAllNearbyNPCs(UserIndex);

		/* '/LASTIP */
		break;

	case eGMCommands_LastIP:
		HandleLastIP(UserIndex);

		/* '/MOTDCAMBIA */
		break;

	case eGMCommands_ChangeMOTD:
		HandleChangeMOTD(UserIndex);

		/* 'ZMOTD */
		break;

	case eGMCommands_SetMOTD:
		HandleSetMOTD(UserIndex);

		/* '/SMSG */
		break;

	case eGMCommands_SystemMessage:
		HandleSystemMessage(UserIndex);

		/* '/ACC */
		break;

	case eGMCommands_CreateNPC:
		HandleCreateNPC(UserIndex);

		/* '/RACC */
		break;

	case eGMCommands_CreateNPCWithRespawn:
		HandleCreateNPCWithRespawn(UserIndex);

		/* '/AI1 - 4 */
		break;

	case eGMCommands_ImperialArmour:
		HandleImperialArmour(UserIndex);

		/* '/AC1 - 4 */
		break;

	case eGMCommands_ChaosArmour:
		HandleChaosArmour(UserIndex);

		/* '/NAVE */
		break;

	case eGMCommands_NavigateToggle:
		HandleNavigateToggle(UserIndex);

		/* '/HABILITAR */
		break;

	case eGMCommands_ServerOpenToUsersToggle:
		HandleServerOpenToUsersToggle(UserIndex);

		/* '/APAGAR */
		break;

	case eGMCommands_TurnOffServer:
		HandleTurnOffServer(UserIndex);

		/* '/CONDEN */
		break;

	case eGMCommands_TurnCriminal:
		HandleTurnCriminal(UserIndex);

		/* '/RAJAR */
		break;

	case eGMCommands_ResetFactions:
		HandleResetFactions(UserIndex);

		/* '/RAJARCLAN */
		break;

	case eGMCommands_RemoveCharFromGuild:
		HandleRemoveCharFromGuild(UserIndex);

		/* '/LASTEMAIL */
		break;

	case eGMCommands_RequestCharMail:
		HandleRequestCharMail(UserIndex);

		/* '/APASS */
		break;

	case eGMCommands_AlterPassword:
		HandleAlterPassword(UserIndex);

		/* '/AEMAIL */
		break;

	case eGMCommands_AlterMail:
		HandleAlterMail(UserIndex);

		/* '/ANAME */
		break;

	case eGMCommands_AlterName:
		HandleAlterName(UserIndex);

		/* '/CENTINELAACTIVADO */
		break;

	case eGMCommands_ToggleCentinelActivated:
		HandleToggleCentinelActivated(UserIndex);

		/* '/DOBACKUP */
		break;

	case eGMCommands_DoBackUp:
		HandleDoBackUp(UserIndex);

		/* '/SHOWCMSG */
		break;

	case eGMCommands_ShowGuildMessages:
		HandleShowGuildMessages(UserIndex);

		/* '/GUARDAMAPA */
		break;

	case eGMCommands_SaveMap:
		HandleSaveMap(UserIndex);

		/* '/MODMAPINFO PK */
		break;

	case eGMCommands_ChangeMapInfoPK:
		HandleChangeMapInfoPK(UserIndex);

		/* '/MODMAPINFO BACKUP */
		break;

	case eGMCommands_ChangeMapInfoBackup:
		HandleChangeMapInfoBackup(UserIndex);

		/* '/MODMAPINFO RESTRINGIR */
		break;

	case eGMCommands_ChangeMapInfoRestricted:
		HandleChangeMapInfoRestricted(UserIndex);

		/* '/MODMAPINFO MAGIASINEFECTO */
		break;

	case eGMCommands_ChangeMapInfoNoMagic:
		HandleChangeMapInfoNoMagic(UserIndex);

		/* '/MODMAPINFO INVISINEFECTO */
		break;

	case eGMCommands_ChangeMapInfoNoInvi:
		HandleChangeMapInfoNoInvi(UserIndex);

		/* '/MODMAPINFO RESUSINEFECTO */
		break;

	case eGMCommands_ChangeMapInfoNoResu:
		HandleChangeMapInfoNoResu(UserIndex);

		/* '/MODMAPINFO TERRENO */
		break;

	case eGMCommands_ChangeMapInfoLand:
		HandleChangeMapInfoLand(UserIndex);

		/* '/MODMAPINFO ZONA */
		break;

	case eGMCommands_ChangeMapInfoZone:
		HandleChangeMapInfoZone(UserIndex);

		/* '/MODMAPINFO ROBONPC */
		break;

	case eGMCommands_ChangeMapInfoStealNpc:
		HandleChangeMapInfoStealNpc(UserIndex);

		/* '/MODMAPINFO OCULTARSINEFECTO */
		break;

	case eGMCommands_ChangeMapInfoNoOcultar:
		HandleChangeMapInfoNoOcultar(UserIndex);

		/* '/MODMAPINFO INVOCARSINEFECTO */
		break;

	case eGMCommands_ChangeMapInfoNoInvocar:
		HandleChangeMapInfoNoInvocar(UserIndex);

		/* '/GRABAR */
		break;

	case eGMCommands_SaveChars:
		HandleSaveChars(UserIndex);

		/* '/BORRAR SOS */
		break;

	case eGMCommands_CleanSOS:
		HandleCleanSOS(UserIndex);

		/* '/SHOW INT */
		break;

	case eGMCommands_ShowServerForm:
		HandleShowServerForm(UserIndex);

		/* '/NOCHE */
		break;

	case eGMCommands_night:
		HandleNight(UserIndex);

		/* '/ECHARTODOSPJS */
		break;

	case eGMCommands_KickAllChars:
		HandleKickAllChars(UserIndex);

		/* '/RELOADNPCS */
		break;

	case eGMCommands_ReloadNPCs:
		HandleReloadNPCs(UserIndex);

		/* '/RELOADSINI */
		break;

	case eGMCommands_ReloadServerIni:
		HandleReloadServerIni(UserIndex);

		/* '/RELOADHECHIZOS */
		break;

	case eGMCommands_ReloadSpells:
		HandleReloadSpells(UserIndex);

		/* '/RELOADOBJ */
		break;

	case eGMCommands_ReloadObjects:
		HandleReloadObjects(UserIndex);

		/* '/REINICIAR */
		break;

	case eGMCommands_Restart:
		HandleRestart(UserIndex);

		/* '/AUTOUPDATE */
		break;

	case eGMCommands_ResetAutoUpdate:
		HandleResetAutoUpdate(UserIndex);

		/* '/CHATCOLOR */
		break;

	case eGMCommands_ChatColor:
		HandleChatColor(UserIndex);

		/* '/IGNORADO */
		break;

	case eGMCommands_Ignored:
		HandleIgnored(UserIndex);

		/* '/SLOT */
		break;

	case eGMCommands_CheckSlot:
		HandleCheckSlot(UserIndex);

		/* '/SETINIVAR LLAVE CLAVE VALOR */
		break;

	case eGMCommands_SetIniVar:
		HandleSetIniVar(UserIndex);

		/* '/CREARPRETORIANOS */
		break;

	case eGMCommands_CreatePretorianClan:
		HandleCreatePretorianClan(UserIndex);

		/* '/ELIMINARPRETORIANOS */
		break;

	case eGMCommands_RemovePretorianClan:
		HandleDeletePretorianClan(UserIndex);

		/* '/DENUNCIAS */
		break;

	case eGMCommands_EnableDenounces:
		HandleEnableDenounces(UserIndex);

		/* '/SHOW DENUNCIAS */
		break;

	case eGMCommands_ShowDenouncesList:
		HandleShowDenouncesList(UserIndex);

		/* '/SETDIALOG */
		break;

	case eGMCommands_SetDialog:
		HandleSetDialog(UserIndex);

		/* '/IMPERSONAR */
		break;

	case eGMCommands_Impersonate:
		HandleImpersonate(UserIndex);

		/* '/MIMETIZAR */
		break;

	case eGMCommands_Imitate:
		HandleImitate(UserIndex);

		break;

	case eGMCommands_RecordAdd:
		HandleRecordAdd(UserIndex);

		break;

	case eGMCommands_RecordAddObs:
		HandleRecordAddObs(UserIndex);

		break;

	case eGMCommands_RecordRemove:
		HandleRecordRemove(UserIndex);

		break;

	case eGMCommands_RecordListRequest:
		HandleRecordListRequest(UserIndex);

		break;

	case eGMCommands_RecordDetailsRequest:
		HandleRecordDetailsRequest(UserIndex);

		break;

	case eGMCommands_HigherAdminsMessage:
		HandleHigherAdminsMessage(UserIndex);

		/* '/ACLAN */
		break;

	case eGMCommands_AlterGuildName:
		HandleAlterGuildName(UserIndex);
		break;

	default:
		throw std::runtime_error("Unkwnown GM Command");
	}
}

/* '' */
/* ' Handles the "Home" message. */
/* ' */
/* ' @param    userIndex The index of the user sending the message. */
void HandleHome(int UserIndex) {
	/* '*************************************************** */
	/* 'Author: Budi */
	/* 'Creation Date: 06/01/2010 */
	/* 'Last Modification: 05/06/10 */
	/* 'Pato - 05/06/10: Add the Ucase$ to prevent problems. */
	/* '*************************************************** */
	UserList[UserIndex].sockctx->incomingData->ReadByte();
	if (UserList[UserIndex].flags.TargetNpcTipo == eNPCType_Gobernador) {
		setHome(UserIndex, static_cast<eCiudad>(Npclist[UserList[UserIndex].flags.TargetNPC].Ciudad),
				UserList[UserIndex].flags.TargetNPC);
	} else {
		if (UserList[UserIndex].flags.Muerto == 1) {
			/* 'Si es un mapa común y no está en cana */
			if ((MapInfo[UserList[UserIndex].Pos.Map].Restringir == eRestrict_restrict_no)
					&& (UserList[UserIndex].Counters.Pena == 0)) {
				if (UserList[UserIndex].flags.Traveling == 0) {
					if (Ciudades[UserList[UserIndex].Hogar].Map != UserList[UserIndex].Pos.Map) {
						goHome(UserIndex);
					} else {
						WriteConsoleMsg(UserIndex, "Ya te encuentras en tu hogar.",
								FontTypeNames_FONTTYPE_INFO);
					}
				} else {
					WriteMultiMessage(UserIndex, eMessages_CancelHome);
					UserList[UserIndex].flags.Traveling = 0;
					UserList[UserIndex].Counters.goHome = 0;
				}
			} else {
				WriteConsoleMsg(UserIndex, "No puedes usar este comando aquí.", FontTypeNames_FONTTYPE_FIGHT);
			}
		} else {
			WriteConsoleMsg(UserIndex, "Debes estar muerto para utilizar este comando.",
					FontTypeNames_FONTTYPE_INFO);
		}
	}
}

/* '' */
/* ' Handles the "LoginExistingChar" message. */
/* ' */
/* ' @param    userIndex The index of the user sending the message. */

void HandleLoginExistingChar(int UserIndex) {
	/* '*************************************************** */
	/* 'Author: Juan Martín Sotuyo Dodero (Maraxus) */
	/* 'Last Modification: 05/17/06 */
	/* ' */
	/* '*************************************************** */
	/* # IF SeguridadAlkon THEN */
	/* # ELSE */
	if (UserList[UserIndex].sockctx->incomingData->length() < 6) {
		throw insufficient_data_error();
		return;
	}
	/* # END IF */

	/* 'This packet contains strings, make a copy of the data to prevent losses if it's not complete yet... */
	clsByteQueue* buffer = UserList[UserIndex].sockctx->incomingData.get();


	/* 'Remove packet ID */
	buffer->ReadByte();

	std::string UserName;
	std::string Password;
	std::string version;

	UserName = buffer->ReadUnicodeString();

	/* # IF SeguridadAlkon THEN */
	/* # ELSE */
	Password = buffer->ReadUnicodeString();
	/* # END IF */

	{
		int vera = buffer->ReadByte();
		int verb = buffer->ReadByte();
		int verc = buffer->ReadByte();

		/* 'Convert version number to string */
		version = vb6::CStr(vera) + "." + vb6::CStr(verb) + "." + vb6::CStr(verc);
	}

	if (!AsciiValidos(UserName)) {
		WriteErrorMsg(UserIndex, "Nombre inválido.");
		FlushBuffer(UserIndex);
		CloseSocket(UserIndex);

		return;
	}

	if (!PersonajeExiste(UserName)) {
		WriteErrorMsg(UserIndex, "El personaje no existe.");
		FlushBuffer(UserIndex);
		CloseSocket(UserIndex);

		return;
	}

	if (BANCheck(UserName)) {
		WriteErrorMsg(UserIndex,
				"Se te ha prohibido la entrada a Argentum Online debido a tu mal comportamiento. Puedes consultar el reglamento y el sistema de soporte desde www.argentumonline.com.ar");
	} else if (!VersionOK(version)) {
		WriteErrorMsg(UserIndex,
				"Esta versión del juego es obsoleta, la versión correcta es la " + ULTIMAVERSION
						+ ". La misma se encuentra disponible en www.argentumonline.com.ar");
	} else {
		ConnectUser(UserIndex, UserName, Password);
	}
}

/* '' */
/* ' Handles the "ThrowDices" message. */
/* ' */
/* ' @param    userIndex The index of the user sending the message. */

void HandleThrowDices(int UserIndex) {
	/* '*************************************************** */
	/* 'Last Modification: 05/17/06 */
	/* 'Author: Juan Martín Sotuyo Dodero (Maraxus) */
	/* ' */
	/* '*************************************************** */
	/* 'Remove packet ID */
	UserList[UserIndex].sockctx->incomingData->ReadByte();

	UserList[UserIndex].Stats.UserAtributos[eAtributos_Fuerza] = MaximoInt(15,
			13 + RandomNumber(0, 3) + RandomNumber(0, 2));
	UserList[UserIndex].Stats.UserAtributos[eAtributos_Agilidad] = MaximoInt(15,
			12 + RandomNumber(0, 3) + RandomNumber(0, 3));
	UserList[UserIndex].Stats.UserAtributos[eAtributos_Inteligencia] = MaximoInt(16,
			13 + RandomNumber(0, 3) + RandomNumber(0, 2));
	UserList[UserIndex].Stats.UserAtributos[eAtributos_Carisma] = MaximoInt(15,
			12 + RandomNumber(0, 3) + RandomNumber(0, 3));
	/* ' [TEMPORAL] 16 + RandomNumber(0, 1) + RandomNumber(0, 1) */
	UserList[UserIndex].Stats.UserAtributos[eAtributos_Constitucion] = 18;

	WriteDiceRoll(UserIndex);
}

/* '' */
/* ' Handles the "LoginNewChar" message. */
/* ' */
/* ' @param    userIndex The index of the user sending the message. */

void HandleLoginNewChar(int UserIndex) {
	/* '*************************************************** */
	/* 'Author: Juan Martín Sotuyo Dodero (Maraxus) */
	/* 'Last Modification: 05/17/06 */
	/* ' */
	/* '*************************************************** */
	/* # IF SeguridadAlkon THEN */
	/* # ELSE */
	if (UserList[UserIndex].sockctx->incomingData->length() < 15) {
		throw insufficient_data_error();
		return;
	}
	/* # END IF */

	/* 'This packet contains strings, make a copy of the data to prevent losses if it's not complete yet... */
	clsByteQueue* buffer = UserList[UserIndex].sockctx->incomingData.get();


	/* 'Remove packet ID */
	buffer->ReadByte();

	std::string UserName;
	std::string Password;
	std::string version;
	eRaza race;
	eGenero gender;
	eCiudad homeland;
	eClass Class;
	int Head;
	std::string mail;

	/* # IF SeguridadAlkon THEN */
	/* # END IF */

	if (PuedeCrearPersonajes == 0) {
		WriteErrorMsg(UserIndex, "La creación de personajes en este servidor se ha deshabilitado.");
		FlushBuffer(UserIndex);
		CloseSocket(UserIndex);

		return;
	}

	if (ServerSoloGMs != 0) {
		WriteErrorMsg(UserIndex,
				"Servidor restringido a administradores. Consulte la página oficial o el foro oficial para más información.");
		FlushBuffer(UserIndex);
		CloseSocket(UserIndex);

		return;
	}

	if (aClon->MaxPersonajes(UserList[UserIndex].ip)) {
		WriteErrorMsg(UserIndex, "Has creado demasiados personajes.");
		FlushBuffer(UserIndex);
		CloseSocket(UserIndex);

		return;
	}

	UserName = buffer->ReadUnicodeString();

	/* # IF SeguridadAlkon THEN */
	/* # ELSE */
	Password = buffer->ReadUnicodeString();

	/* # END IF */

	{
		int vera = buffer->ReadByte();
		int verb = buffer->ReadByte();
		int verc = buffer->ReadByte();

		/* 'Convert version number to string */ /* FIXME: Undefined order! */
		version = vb6::CStr(vera) + "." + vb6::CStr(verb) + "." + vb6::CStr(verc);
	}

	/* # IF SeguridadAlkon THEN */
	/* # END IF */

	race = static_cast<eRaza>(buffer->ReadByte());
	gender = static_cast<eGenero>(buffer->ReadByte());
	Class = static_cast<eClass>(buffer->ReadByte());
	Head = buffer->ReadInteger();
	mail = buffer->ReadUnicodeString();
	homeland = static_cast<eCiudad>(buffer->ReadByte());

	/* # IF SeguridadAlkon THEN */
	/* # END IF */

	if (!VersionOK(version)) {
		WriteErrorMsg(UserIndex,
				"Esta versión del juego es obsoleta, la versión correcta es la " + ULTIMAVERSION
						+ ". La misma se encuentra disponible en www.dakara.com.ar");
	} else {
		ConnectNewUser(UserIndex, UserName, Password, race, gender, Class, mail, homeland, Head);
	}
	/* # IF SeguridadAlkon THEN */
	/* # END IF */

	/* 'If we got here then packet is complete, copy data back to original queue */

}

/* '' */
/* ' Handles the "Talk" message. */
/* ' */
/* ' @param    userIndex The index of the user sending the message. */

void HandleTalk(int UserIndex) {
	/* '*************************************************** */
	/* 'Author: Juan Martín Sotuyo Dodero (Maraxus) */
	/* 'Last Modification: 13/01/2010 */
	/* '15/07/2009: ZaMa - Now invisible admins talk by console. */
	/* '23/09/2009: ZaMa - Now invisible admins can't send empty chat. */
	/* '13/01/2010: ZaMa - Now hidden on boat pirats recover the proper boat body. */
	/* '*************************************************** */
	if (UserList[UserIndex].sockctx->incomingData->length() < 3) {
		throw insufficient_data_error();
		return;
	}

	/* 'This packet contains strings, make a copy of the data to prevent losses if it's not complete yet... */
	clsByteQueue* buffer = UserList[UserIndex].sockctx->incomingData.get();


	/* 'Remove packet ID */
	buffer->ReadByte();

	std::string Chat;

	Chat = buffer->ReadUnicodeString();

	/* '[Consejeros & GMs] */
	if (UserTieneAlgunPrivilegios(UserIndex, PlayerType_Consejero, PlayerType_SemiDios)) {
		LogGM(UserList[UserIndex].Name, "Dijo: " + Chat);
	}

	/* 'I see you.... */
	if (UserList[UserIndex].flags.Oculto > 0) {
		UserList[UserIndex].flags.Oculto = 0;
		UserList[UserIndex].Counters.TiempoOculto = 0;

		if (UserList[UserIndex].flags.Navegando == 1) {
			if (UserList[UserIndex].clase == eClass_Pirat) {
				/* ' Pierde la apariencia de fragata fantasmal */
				ToggleBoatBody(UserIndex);
				WriteConsoleMsg(UserIndex, "¡Has recuperado tu apariencia normal!",
						FontTypeNames_FONTTYPE_INFO);
				ChangeUserChar(UserIndex, UserList[UserIndex].Char.body, UserList[UserIndex].Char.Head,
						UserList[UserIndex].Char.heading, NingunArma, NingunEscudo, NingunCasco);
			}
		} else {
			if (UserList[UserIndex].flags.invisible == 0) {
				SetInvisible(UserIndex, UserList[UserIndex].Char.CharIndex, false);
				WriteConsoleMsg(UserIndex, "¡Has vuelto a ser visible!", FontTypeNames_FONTTYPE_INFO);
			}
		}
	}

	if (vb6::LenB(Chat) != 0) {
		/* 'Analize chat... */
		ParseChat(Chat);

		if (!(UserList[UserIndex].flags.AdminInvisible == 1)) {
			if (UserList[UserIndex].flags.Muerto == 1) {
				SendData(SendTarget_ToDeadArea, UserIndex,
						PrepareMessageChatOverHead(Chat, UserList[UserIndex].Char.CharIndex,
								CHAT_COLOR_DEAD_CHAR));
			} else {
				SendData(SendTarget_ToPCArea, UserIndex,
						PrepareMessageChatOverHead(Chat, UserList[UserIndex].Char.CharIndex,
								UserList[UserIndex].flags.ChatColor));
			}
		} else {
			if (vb6::RTrim(Chat) != "") {
				SendData(SendTarget_ToPCArea, UserIndex,
						PrepareMessageConsoleMsg("Gm> " + Chat, FontTypeNames_FONTTYPE_GM));
			}
		}
	}

	/* 'If we got here then packet is complete, copy data back to original queue */

}

/* '' */
/* ' Handles the "Yell" message. */
/* ' */
/* ' @param    userIndex The index of the user sending the message. */

void HandleYell(int UserIndex) {
	/* '*************************************************** */
	/* 'Author: Juan Martín Sotuyo Dodero (Maraxus) */
	/* 'Last Modification: 13/01/2010 (ZaMa) */
	/* '15/07/2009: ZaMa - Now invisible admins yell by console. */
	/* '13/01/2010: ZaMa - Now hidden on boat pirats recover the proper boat body. */
	/* '*************************************************** */
	if (UserList[UserIndex].sockctx->incomingData->length() < 3) {
		throw insufficient_data_error();
		return;
	}

	/* 'This packet contains strings, make a copy of the data to prevent losses if it's not complete yet... */
	clsByteQueue* buffer = UserList[UserIndex].sockctx->incomingData.get();


	/* 'Remove packet ID */
	buffer->ReadByte();

	std::string Chat;

	Chat = buffer->ReadUnicodeString();

	/* '[Consejeros & GMs] */
	if (UserTieneAlgunPrivilegios(UserIndex, PlayerType_Consejero, PlayerType_SemiDios)) {
		LogGM(UserList[UserIndex].Name, "Grito: " + Chat);
	}

	/* 'I see you.... */
	if (UserList[UserIndex].flags.Oculto > 0) {
		UserList[UserIndex].flags.Oculto = 0;
		UserList[UserIndex].Counters.TiempoOculto = 0;

		if (UserList[UserIndex].flags.Navegando == 1) {
			if (UserList[UserIndex].clase == eClass_Pirat) {
				/* ' Pierde la apariencia de fragata fantasmal */
				ToggleBoatBody(UserIndex);
				WriteConsoleMsg(UserIndex, "¡Has recuperado tu apariencia normal!",
						FontTypeNames_FONTTYPE_INFO);
				ChangeUserChar(UserIndex, UserList[UserIndex].Char.body, UserList[UserIndex].Char.Head,
						UserList[UserIndex].Char.heading, NingunArma, NingunEscudo, NingunCasco);
			}
		} else {
			if (UserList[UserIndex].flags.invisible == 0) {
				SetInvisible(UserIndex, UserList[UserIndex].Char.CharIndex, false);
				WriteConsoleMsg(UserIndex, "¡Has vuelto a ser visible!", FontTypeNames_FONTTYPE_INFO);
			}
		}
	}

	if (vb6::LenB(Chat) != 0) {
		/* 'Analize chat... */
		ParseChat(Chat);

		if (UserTieneAlgunPrivilegios(UserIndex, PlayerType_User)) {
			if (UserList[UserIndex].flags.Muerto == 1) {
				SendData(SendTarget_ToDeadArea, UserIndex,
						PrepareMessageChatOverHead(Chat, UserList[UserIndex].Char.CharIndex,
								CHAT_COLOR_DEAD_CHAR));
			} else {
				SendData(SendTarget_ToPCArea, UserIndex,
						PrepareMessageChatOverHead(Chat, UserList[UserIndex].Char.CharIndex, vbRed));
			}
		} else {
			if (!(UserList[UserIndex].flags.AdminInvisible == 1)) {
				SendData(SendTarget_ToPCArea, UserIndex,
						PrepareMessageChatOverHead(Chat, UserList[UserIndex].Char.CharIndex,
								CHAT_COLOR_GM_YELL));
			} else {
				SendData(SendTarget_ToPCArea, UserIndex,
						PrepareMessageConsoleMsg("Gm> " + Chat, FontTypeNames_FONTTYPE_GM));
			}
		}
	}

	/* 'If we got here then packet is complete, copy data back to original queue */

}

/* '' */
/* ' Handles the "Whisper" message. */
/* ' */
/* ' @param    userIndex The index of the user sending the message. */

void HandleWhisper(int UserIndex) {
	/* '*************************************************** */
	/* 'Author: Juan Martín Sotuyo Dodero (Maraxus) */
	/* 'Last Modification: 03/12/2010 */
	/* '28/05/2009: ZaMa - Now it doesn't appear any message when private talking to an invisible admin */
	/* '15/07/2009: ZaMa - Now invisible admins wisper by console. */
	/* '03/12/2010: Enanoh - Agregué susurro a Admins en modo consulta y Los Dioses pueden susurrar en ciertos casos. */
	/* '*************************************************** */
	if (UserList[UserIndex].sockctx->incomingData->length() < 5) {
		throw insufficient_data_error();
		return;
	}

	/* 'This packet contains strings, make a copy of the data to prevent losses if it's not complete yet... */
	clsByteQueue* buffer = UserList[UserIndex].sockctx->incomingData.get();


	/* 'Remove packet ID */
	buffer->ReadByte();

	std::string Chat;
	int TargetUserIndex;
	std::string TargetName;

	TargetName = buffer->ReadUnicodeString();
	Chat = buffer->ReadUnicodeString();

	if (UserList[UserIndex].flags.Muerto) {
		WriteConsoleMsg(UserIndex,
				"¡¡Estás muerto!! Los muertos no pueden comunicarse con el mundo de los vivos. ",
				FontTypeNames_FONTTYPE_INFO);
	} else {
		/* ' Offline? */
		TargetUserIndex = NameIndex(TargetName);
		if (TargetUserIndex == INVALID_INDEX) {
			/* ' Admin? */
			if (EsGmChar(TargetName)) {
				WriteConsoleMsg(UserIndex, "No puedes susurrarle a los Administradores.",
						FontTypeNames_FONTTYPE_INFO);
				/* ' Whisperer admin? (Else say nothing) */
			} else if (UserTieneAlgunPrivilegios(UserIndex, PlayerType_Dios, PlayerType_Admin)) {
				WriteConsoleMsg(UserIndex, "Usuario inexistente.", FontTypeNames_FONTTYPE_INFO);
			}

			/* ' Online */
		} else {
			/* ' Consejeros, semis y usuarios no pueden susurrar a dioses (Salvo en consulta) */
			if (UserTieneAlgunPrivilegios(TargetUserIndex, PlayerType_Dios, PlayerType_Admin)
					&& UserTieneAlgunPrivilegios(UserIndex, PlayerType_User, PlayerType_Consejero, PlayerType_SemiDios)
					&& !UserList[UserIndex].flags.EnConsulta) {

				/* ' No puede */
				WriteConsoleMsg(UserIndex, "No puedes susurrarle a los Administradores.",
						FontTypeNames_FONTTYPE_INFO);

				/* ' Usuarios no pueden susurrar a semis o conses (Salvo en consulta) */
			} else if (UserTieneAlgunPrivilegios(UserIndex, PlayerType_User) && !UserTieneAlgunPrivilegios(TargetUserIndex, PlayerType_User)
					&& !UserList[UserIndex].flags.EnConsulta) {

				/* ' No puede */
				WriteConsoleMsg(UserIndex, "No puedes susurrarle a los Administradores.",
						FontTypeNames_FONTTYPE_INFO);

				/* ' En rango? (Los dioses pueden susurrar a distancia) */
			} else if (!EstaPCarea(UserIndex, TargetUserIndex)
					&& !UserTieneAlgunPrivilegios(UserIndex, PlayerType_Dios, PlayerType_Admin)) {

				/* ' No se puede susurrar a admins fuera de su rango */
				if (!UserTieneAlgunPrivilegios(TargetUserIndex, PlayerType_User)
						&& !UserTieneAlgunPrivilegios(UserIndex, PlayerType_Dios, PlayerType_Admin)) {
					WriteConsoleMsg(UserIndex, "No puedes susurrarle a los Administradores.",
							FontTypeNames_FONTTYPE_INFO);

					/* ' Whisperer admin? (Else say nothing) */
				} else if (UserTieneAlgunPrivilegios(UserIndex, PlayerType_Dios, PlayerType_Admin)) {
					WriteConsoleMsg(UserIndex, "Estás muy lejos del usuario.", FontTypeNames_FONTTYPE_INFO);
				}
			} else {
				/* '[Consejeros & GMs] */
				if (UserTieneAlgunPrivilegios(UserIndex, PlayerType_Consejero, PlayerType_SemiDios)) {
					LogGM(UserList[UserIndex].Name,
							"Le susurro a '" + UserList[TargetUserIndex].Name + "' " + Chat);

					/* ' Usuarios a administradores */
				} else if (UserTieneAlgunPrivilegios(UserIndex, PlayerType_User) && UserTieneAlgunPrivilegios(TargetUserIndex,  PlayerType_User)) {
					LogGM(UserList[TargetUserIndex].Name,
							UserList[UserIndex].Name + " le susurro en consulta: " + Chat);
				}

				if (vb6::LenB(Chat) != 0) {
					/* 'Analize chat... */
					ParseChat(Chat);

					/* ' Dios susurrando a distancia */
					if (!EstaPCarea(UserIndex, TargetUserIndex)
							&& UserTieneAlgunPrivilegios(UserIndex, PlayerType_Dios, PlayerType_Admin)) {

						WriteConsoleMsg(UserIndex, "Susurraste> " + Chat, FontTypeNames_FONTTYPE_GM);
						WriteConsoleMsg(TargetUserIndex, "Gm susurra> " + Chat, FontTypeNames_FONTTYPE_GM);

					} else if (!(UserList[UserIndex].flags.AdminInvisible == 1)) {
						WriteChatOverHead(UserIndex, Chat, UserList[UserIndex].Char.CharIndex, vbBlue);
						WriteChatOverHead(TargetUserIndex, Chat, UserList[UserIndex].Char.CharIndex, vbBlue);
						FlushBuffer(TargetUserIndex);

						/* '[CDT 17-02-2004] */
						if (UserTieneAlgunPrivilegios(UserIndex, PlayerType_User, PlayerType_Consejero)) {
							SendData(SendTarget_ToAdminsAreaButConsejeros, UserIndex,
									PrepareMessageChatOverHead(
											"A " + UserList[TargetUserIndex].Name + "> " + Chat,
											UserList[UserIndex].Char.CharIndex, vbYellow));
						}
					} else {
						WriteConsoleMsg(UserIndex, "Susurraste> " + Chat, FontTypeNames_FONTTYPE_GM);
						if (UserIndex != TargetUserIndex) {
							WriteConsoleMsg(TargetUserIndex, "Gm susurra> " + Chat,
									FontTypeNames_FONTTYPE_GM);
						}

						if (UserTieneAlgunPrivilegios(UserIndex, PlayerType_User, PlayerType_Consejero)) {
							SendData(SendTarget_ToAdminsAreaButConsejeros, UserIndex,
									PrepareMessageConsoleMsg(
											"Gm dijo a " + UserList[TargetUserIndex].Name + "> " + Chat,
											FontTypeNames_FONTTYPE_GM));
						}
					}
				}
			}
		}
	}

	/* 'If we got here then packet is complete, copy data back to original queue */

}

/* '' */
/* ' Handles the "Walk" message. */
/* ' */
/* ' @param    userIndex The index of the user sending the message. */

void HandleWalk(int UserIndex) {
	/* '*************************************************** */
	/* 'Author: Juan Martín Sotuyo Dodero (Maraxus) */
	/* 'Last Modification: 13/01/2010 (ZaMa) */
	/* '11/19/09 Pato - Now the class bandit can walk hidden. */
	/* '13/01/2010: ZaMa - Now hidden on boat pirats recover the proper boat body. */
	/* '*************************************************** */
	if (UserList[UserIndex].sockctx->incomingData->length() < 2) {
		throw insufficient_data_error();
		return;
	}

	int dummy;
	int TempTick;
	eHeading heading;

	/* 'Remove packet ID */
	UserList[UserIndex].sockctx->incomingData->ReadByte();

	heading = static_cast<eHeading>(UserList[UserIndex].sockctx->incomingData->ReadByte());

	/* 'Prevent SpeedHack */
	if (UserList[UserIndex].flags.TimesWalk >= 30) {
		TempTick = vb6::GetTickCount();
		dummy = getInterval(TempTick, UserList[UserIndex].flags.StartWalk);

		/* ' 5800 is actually less than what would be needed in perfect conditions to take 30 steps */
		/* '(it's about 193 ms per step against the over 200 needed in perfect conditions) */
		if (dummy < 5800) {
			if (getInterval(TempTick, UserList[UserIndex].flags.CountSH) > 30000) {
				UserList[UserIndex].flags.CountSH = 0;
			}

			if (UserList[UserIndex].flags.CountSH != 0) {
				if (dummy != 0) {
					dummy = 126000 / dummy;
				}

				LogHackAttemp("Tramposo SH: " + UserList[UserIndex].Name + " , " + vb6::CStr(dummy));
				SendData(SendTarget_ToAdmins, 0,
						PrepareMessageConsoleMsg(
								"Servidor> " + UserList[UserIndex].Name
										+ " ha sido echado por el servidor por posible uso de SH.",
								FontTypeNames_FONTTYPE_SERVER));
				CloseSocket(UserIndex);

				return;
			} else {
				UserList[UserIndex].flags.CountSH = TempTick;
			}
		}
		UserList[UserIndex].flags.StartWalk = TempTick;
		UserList[UserIndex].flags.TimesWalk = 0;
	}

	UserList[UserIndex].flags.TimesWalk = UserList[UserIndex].flags.TimesWalk + 1;

	/* 'If exiting, cancel */
	CancelExit(UserIndex);

	/* 'TODO: Debería decirle por consola que no puede? */
	/* 'Esta usando el /HOGAR, no se puede mover */
	if (UserList[UserIndex].flags.Traveling == 1) {
		return;
	}

	if (UserList[UserIndex].flags.Paralizado == 0) {
		if (UserList[UserIndex].flags.Meditando) {
			/* 'Stop meditating, next action will start movement. */
			UserList[UserIndex].flags.Meditando = false;
			UserList[UserIndex].Char.FX = 0;
			UserList[UserIndex].Char.loops = 0;

			WriteMeditateToggle(UserIndex);
			WriteConsoleMsg(UserIndex, "Dejas de meditar.", FontTypeNames_FONTTYPE_INFO);

			SendData(SendTarget_ToPCArea, UserIndex,
					PrepareMessageCreateFX(UserList[UserIndex].Char.CharIndex, 0, 0));
		} else {
			/* 'Move user */
			MoveUserChar(UserIndex, heading);

			/* 'Stop resting if needed */
			if (UserList[UserIndex].flags.Descansar) {
				UserList[UserIndex].flags.Descansar = false;

				WriteRestOK(UserIndex);
				WriteConsoleMsg(UserIndex, "Has dejado de descansar.", FontTypeNames_FONTTYPE_INFO);
			}
		}
		/* 'paralized */
	} else {
		if (UserList[UserIndex].flags.UltimoMensaje != 1) {
			UserList[UserIndex].flags.UltimoMensaje = 1;

			WriteConsoleMsg(UserIndex, "No puedes moverte porque estás paralizado.",
					FontTypeNames_FONTTYPE_INFO);
		}

		UserList[UserIndex].flags.CountSH = 0;
	}

	/* 'Can't move while hidden except he is a thief */
	if (UserList[UserIndex].flags.Oculto == 1 && UserList[UserIndex].flags.AdminInvisible == 0) {
		if (UserList[UserIndex].clase != eClass_Thief && UserList[UserIndex].clase != eClass_Bandit) {
			UserList[UserIndex].flags.Oculto = 0;
			UserList[UserIndex].Counters.TiempoOculto = 0;

			if (UserList[UserIndex].flags.Navegando == 1) {
				if (UserList[UserIndex].clase == eClass_Pirat) {
					/* ' Pierde la apariencia de fragata fantasmal */
					ToggleBoatBody(UserIndex);
					WriteConsoleMsg(UserIndex, "¡Has recuperado tu apariencia normal!",
							FontTypeNames_FONTTYPE_INFO);
					ChangeUserChar(UserIndex, UserList[UserIndex].Char.body, UserList[UserIndex].Char.Head,
							UserList[UserIndex].Char.heading, NingunArma, NingunEscudo, NingunCasco);
				}
			} else {
				/* 'If not under a spell effect, show char */
				if (UserList[UserIndex].flags.invisible == 0) {
					WriteConsoleMsg(UserIndex, "Has vuelto a ser visible.", FontTypeNames_FONTTYPE_INFO);
					SetInvisible(UserIndex, UserList[UserIndex].Char.CharIndex, false);
				}
			}
		}
	}
}

/* '' */
/* ' Handles the "RequestPositionUpdate" message. */
/* ' */
/* ' @param    userIndex The index of the user sending the message. */

void HandleRequestPositionUpdate(int UserIndex) {
	/* '*************************************************** */
	/* 'Author: Juan Martín Sotuyo Dodero (Maraxus) */
	/* 'Last Modification: 05/17/06 */
	/* ' */
	/* '*************************************************** */
	/* 'Remove packet ID */
	UserList[UserIndex].sockctx->incomingData->ReadByte();

	WritePosUpdate(UserIndex);
}

/* '' */
/* ' Handles the "Attack" message. */
/* ' */
/* ' @param    userIndex The index of the user sending the message. */

void HandleAttack(int UserIndex) {
	/* '*************************************************** */
	/* 'Author: Juan Martín Sotuyo Dodero (Maraxus) */
	/* 'Last Modification: 13/01/2010 */
	/* 'Last Modified By: ZaMa */
	/* '10/01/2008: Tavo - Se cancela la salida del juego si el user esta saliendo. */
	/* '13/11/2009: ZaMa - Se cancela el estado no atacable al atcar. */
	/* '13/01/2010: ZaMa - Now hidden on boat pirats recover the proper boat body. */
	/* '*************************************************** */
	/* 'Remove packet ID */
	UserList[UserIndex].sockctx->incomingData->ReadByte();

	/* 'If dead, can't attack */
	if (UserList[UserIndex].flags.Muerto == 1) {
		WriteConsoleMsg(UserIndex, "¡¡Estás muerto!!", FontTypeNames_FONTTYPE_INFO);
		return;
	}

	/* 'If user meditates, can't attack */
	if (UserList[UserIndex].flags.Meditando) {
		return;
	}

	/* 'If equiped weapon is ranged, can't attack this way */
	if (UserList[UserIndex].Invent.WeaponEqpObjIndex > 0) {
		if (ObjData[UserList[UserIndex].Invent.WeaponEqpObjIndex].proyectil == 1) {
			WriteConsoleMsg(UserIndex, "No puedes usar así este arma.", FontTypeNames_FONTTYPE_INFO);
			return;
		}
	}

	/* 'Admins can't attack. */
	if (!UserTieneAlgunPrivilegios(UserIndex, PlayerType_User)) {
		return;
	}

	/* 'If exiting, cancel */
	CancelExit(UserIndex);

	/* 'Attack! */
	UsuarioAtaca(UserIndex);

	/* 'Now you can be atacked */
	UserList[UserIndex].flags.NoPuedeSerAtacado = false;

	/* 'I see you... */
	if (UserList[UserIndex].flags.Oculto > 0 && UserList[UserIndex].flags.AdminInvisible == 0) {
		UserList[UserIndex].flags.Oculto = 0;
		UserList[UserIndex].Counters.TiempoOculto = 0;

		if (UserList[UserIndex].flags.Navegando == 1) {
			if (UserList[UserIndex].clase == eClass_Pirat) {
				/* ' Pierde la apariencia de fragata fantasmal */
				ToggleBoatBody(UserIndex);
				WriteConsoleMsg(UserIndex, "¡Has recuperado tu apariencia normal!",
						FontTypeNames_FONTTYPE_INFO);
				ChangeUserChar(UserIndex, UserList[UserIndex].Char.body, UserList[UserIndex].Char.Head,
						UserList[UserIndex].Char.heading, NingunArma, NingunEscudo, NingunCasco);
			}
		} else {
			if (UserList[UserIndex].flags.invisible == 0) {
				SetInvisible(UserIndex, UserList[UserIndex].Char.CharIndex, false);
				WriteConsoleMsg(UserIndex, "¡Has vuelto a ser visible!", FontTypeNames_FONTTYPE_INFO);
			}
		}
	}
}

/* '' */
/* ' Handles the "PickUp" message. */
/* ' */
/* ' @param    userIndex The index of the user sending the message. */

void HandlePickUp(int UserIndex) {
	/* '*************************************************** */
	/* 'Author: Juan Martín Sotuyo Dodero (Maraxus) */
	/* 'Last Modification: 07/25/09 */
	/* '02/26/2006: Marco - Agregué un checkeo por si el usuario trata de agarrar un item mientras comercia. */
	/* '*************************************************** */
	/* 'Remove packet ID */
	UserList[UserIndex].sockctx->incomingData->ReadByte();

	/* 'If dead, it can't pick up objects */
	if (UserList[UserIndex].flags.Muerto == 1) {
		return;
	}

	/* 'If user is trading items and attempts to pickup an item, he's cheating, so we kick him. */
	if (UserList[UserIndex].flags.Comerciando) {
		return;
	}

	/* 'Lower rank administrators can't pick up items */
	if (UserTieneAlgunPrivilegios(UserIndex, PlayerType_Consejero)) {
		if (!UserTieneAlgunPrivilegios(UserIndex, PlayerType_RoleMaster)) {
			WriteConsoleMsg(UserIndex, "No puedes tomar ningún objeto.", FontTypeNames_FONTTYPE_INFO);
			return;
		}
	}

	GetObj(UserIndex);
}

/* '' */
/* ' Handles the "SafeToggle" message. */
/* ' */
/* ' @param    userIndex The index of the user sending the message. */

void HandleSafeToggle(int UserIndex) {
	/* '*************************************************** */
	/* 'Author: Juan Martín Sotuyo Dodero (Maraxus) */
	/* 'Last Modification: 05/17/06 */
	/* ' */
	/* '*************************************************** */
	/* 'Remove packet ID */
	UserList[UserIndex].sockctx->incomingData->ReadByte();

	if (UserList[UserIndex].flags.Seguro) {
		/* 'Call WriteSafeModeOff(UserIndex) */
		WriteMultiMessage(UserIndex, eMessages_SafeModeOff);
	} else {
		/* 'Call WriteSafeModeOn(UserIndex) */
		WriteMultiMessage(UserIndex, eMessages_SafeModeOn);
	}

	UserList[UserIndex].flags.Seguro = !UserList[UserIndex].flags.Seguro;
}

/* '' */
/* ' Handles the "ResuscitationSafeToggle" message. */
/* ' */
/* ' @param    userIndex The index of the user sending the message. */

void HandleResuscitationToggle(int UserIndex) {
	/* '*************************************************** */
	/* 'Author: Rapsodius */
	/* 'Creation Date: 10/10/07 */
	/* '*************************************************** */
	UserList[UserIndex].sockctx->incomingData->ReadByte();

	UserList[UserIndex].flags.SeguroResu = !UserList[UserIndex].flags.SeguroResu;

	if (UserList[UserIndex].flags.SeguroResu) {
		/* 'Call WriteResuscitationSafeOn(UserIndex) */
		WriteMultiMessage(UserIndex, eMessages_ResuscitationSafeOn);
	} else {
		/* 'Call WriteResuscitationSafeOff(UserIndex) */
		WriteMultiMessage(UserIndex, eMessages_ResuscitationSafeOff);
	}
}

/* '' */
/* ' Handles the "RequestGuildLeaderInfo" message. */
/* ' */
/* ' @param    userIndex The index of the user sending the message. */

void HandleRequestGuildLeaderInfo(int UserIndex) {
	/* '*************************************************** */
	/* 'Author: Juan Martín Sotuyo Dodero (Maraxus) */
	/* 'Last Modification: 05/17/06 */
	/* ' */
	/* '*************************************************** */
	/* 'Remove packet ID */
	UserList[UserIndex].sockctx->incomingData->ReadByte();

	SendGuildLeaderInfo(UserIndex);
}

/* '' */
/* ' Handles the "RequestAtributes" message. */
/* ' */
/* ' @param    userIndex The index of the user sending the message. */

void HandleRequestAtributes(int UserIndex) {
	/* '*************************************************** */
	/* 'Author: Juan Martín Sotuyo Dodero (Maraxus) */
	/* 'Last Modification: 05/17/06 */
	/* ' */
	/* '*************************************************** */
	/* 'Remove packet ID */
	UserList[UserIndex].sockctx->incomingData->ReadByte();

	WriteAttributes(UserIndex);
}

/* '' */
/* ' Handles the "RequestFame" message. */
/* ' */
/* ' @param    userIndex The index of the user sending the message. */

void HandleRequestFame(int UserIndex) {
	/* '*************************************************** */
	/* 'Author: Juan Martín Sotuyo Dodero (Maraxus) */
	/* 'Last Modification: 05/17/06 */
	/* ' */
	/* '*************************************************** */
	/* 'Remove packet ID */
	UserList[UserIndex].sockctx->incomingData->ReadByte();

	EnviarFama(UserIndex);
}

/* '' */
/* ' Handles the "RequestSkills" message. */
/* ' */
/* ' @param    userIndex The index of the user sending the message. */

void HandleRequestSkills(int UserIndex) {
	/* '*************************************************** */
	/* 'Author: Juan Martín Sotuyo Dodero (Maraxus) */
	/* 'Last Modification: 05/17/06 */
	/* ' */
	/* '*************************************************** */
	/* 'Remove packet ID */
	UserList[UserIndex].sockctx->incomingData->ReadByte();

	WriteSendSkills(UserIndex);
}

/* '' */
/* ' Handles the "RequestMiniStats" message. */
/* ' */
/* ' @param    userIndex The index of the user sending the message. */

void HandleRequestMiniStats(int UserIndex) {
	/* '*************************************************** */
	/* 'Author: Juan Martín Sotuyo Dodero (Maraxus) */
	/* 'Last Modification: 05/17/06 */
	/* ' */
	/* '*************************************************** */
	/* 'Remove packet ID */
	UserList[UserIndex].sockctx->incomingData->ReadByte();

	WriteMiniStats(UserIndex);
}

/* '' */
/* ' Handles the "CommerceEnd" message. */
/* ' */
/* ' @param    userIndex The index of the user sending the message. */

void HandleCommerceEnd(int UserIndex) {
	/* '*************************************************** */
	/* 'Author: Juan Martín Sotuyo Dodero (Maraxus) */
	/* 'Last Modification: 05/17/06 */
	/* ' */
	/* '*************************************************** */
	/* 'Remove packet ID */
	UserList[UserIndex].sockctx->incomingData->ReadByte();

	/* 'User quits commerce mode */
	UserList[UserIndex].flags.Comerciando = false;
	WriteCommerceEnd(UserIndex);
}

/* '' */
/* ' Handles the "UserCommerceEnd" message. */
/* ' */
/* ' @param    userIndex The index of the user sending the message. */

void HandleUserCommerceEnd(int UserIndex) {
	/* '*************************************************** */
	/* 'Author: Juan Martín Sotuyo Dodero (Maraxus) */
	/* 'Last Modification: 11/03/2010 */
	/* '11/03/2010: ZaMa - Le avisa por consola al que cencela que dejo de comerciar. */
	/* '*************************************************** */
	/* 'Remove packet ID */
	UserList[UserIndex].sockctx->incomingData->ReadByte();

	/* 'Quits commerce mode with user */
	if (UserList[UserIndex].ComUsu.DestUsu > 0) {
		if (UserList[UserList[UserIndex].ComUsu.DestUsu].ComUsu.DestUsu == UserIndex) {
			WriteConsoleMsg(UserList[UserIndex].ComUsu.DestUsu,
					UserList[UserIndex].Name + " ha dejado de comerciar con vos.",
					FontTypeNames_FONTTYPE_TALK);
			FinComerciarUsu(UserList[UserIndex].ComUsu.DestUsu);

			/* 'Send data in the outgoing buffer of the other user */
			FlushBuffer(UserList[UserIndex].ComUsu.DestUsu);
		}
	}

	FinComerciarUsu(UserIndex);
	WriteConsoleMsg(UserIndex, "Has dejado de comerciar.", FontTypeNames_FONTTYPE_TALK);
}

/* '' */
/* ' Handles the "UserCommerceConfirm" message. */
/* ' */
/* ' @param    userIndex The index of the user sending the message. */
void HandleUserCommerceConfirm(int UserIndex) {
	/* '*************************************************** */
	/* 'Author: ZaMa */
	/* 'Last Modification: 14/12/2009 */
	/* ' */
	/* '*************************************************** */

	/* 'Remove packet ID */
	UserList[UserIndex].sockctx->incomingData->ReadByte();

	/* 'Validate the commerce */
	if (PuedeSeguirComerciando(UserIndex)) {
		/* 'Tell the other user the confirmation of the offer */
		WriteUserOfferConfirm(UserList[UserIndex].ComUsu.DestUsu);
		UserList[UserIndex].ComUsu.Confirmo = true;
	}

}

void HandleCommerceChat(int UserIndex) {
	/* '*************************************************** */
	/* 'Author: ZaMa */
	/* 'Last Modification: 03/12/2009 */
	/* ' */
	/* '*************************************************** */
	if (UserList[UserIndex].sockctx->incomingData->length() < 3) {
		throw insufficient_data_error();
		return;
	}

	/* 'This packet contains strings, make a copy of the data to prevent losses if it's not complete yet... */
	clsByteQueue* buffer = UserList[UserIndex].sockctx->incomingData.get();


	/* 'Remove packet ID */
	buffer->ReadByte();

	std::string Chat;

	Chat = buffer->ReadUnicodeString();

	if (vb6::LenB(Chat) != 0) {
		if (PuedeSeguirComerciando(UserIndex)) {
			/* 'Analize chat... */
			ParseChat(Chat);

			Chat = UserList[UserIndex].Name + "> " + Chat;
			WriteCommerceChat(UserIndex, Chat, FontTypeNames_FONTTYPE_PARTY);
			WriteCommerceChat(UserList[UserIndex].ComUsu.DestUsu, Chat, FontTypeNames_FONTTYPE_PARTY);
		}
	}

	/* 'If we got here then packet is complete, copy data back to original queue */


}

/* '' */
/* ' Handles the "BankEnd" message. */
/* ' */
/* ' @param    userIndex The index of the user sending the message. */

void HandleBankEnd(int UserIndex) {
	/* '*************************************************** */
	/* 'Author: Juan Martín Sotuyo Dodero (Maraxus) */
	/* 'Last Modification: 05/17/06 */
	/* ' */
	/* '*************************************************** */
	/* 'Remove packet ID */
	UserList[UserIndex].sockctx->incomingData->ReadByte();

	/* 'User exits banking mode */
	UserList[UserIndex].flags.Comerciando = false;
	WriteBankEnd(UserIndex);
}

/* '' */
/* ' Handles the "UserCommerceOk" message. */
/* ' */
/* ' @param    userIndex The index of the user sending the message. */

void HandleUserCommerceOk(int UserIndex) {
	/* '*************************************************** */
	/* 'Author: Juan Martín Sotuyo Dodero (Maraxus) */
	/* 'Last Modification: 05/17/06 */
	/* ' */
	/* '*************************************************** */
	/* 'Remove packet ID */
	UserList[UserIndex].sockctx->incomingData->ReadByte();

	/* 'Trade accepted */
	AceptarComercioUsu(UserIndex);
}

/* '' */
/* ' Handles the "UserCommerceReject" message. */
/* ' */
/* ' @param    userIndex The index of the user sending the message. */

void HandleUserCommerceReject(int UserIndex) {
	/* '*************************************************** */
	/* 'Author: Juan Martín Sotuyo Dodero (Maraxus) */
	/* 'Last Modification: 05/17/06 */
	/* ' */
	/* '*************************************************** */
	int otherUser;

	/* 'Remove packet ID */
	UserList[UserIndex].sockctx->incomingData->ReadByte();

	otherUser = UserList[UserIndex].ComUsu.DestUsu;

	/* 'Offer rejected */
	if (otherUser > 0) {
		if (UserList[otherUser].flags.UserLogged) {
			WriteConsoleMsg(otherUser, UserList[UserIndex].Name + " ha rechazado tu oferta.",
					FontTypeNames_FONTTYPE_TALK);
			FinComerciarUsu(otherUser);

			/* 'Send data in the outgoing buffer of the other user */
			FlushBuffer(otherUser);
		}
	}

	WriteConsoleMsg(UserIndex, "Has rechazado la oferta del otro usuario.", FontTypeNames_FONTTYPE_TALK);
	FinComerciarUsu(UserIndex);
}

/* '' */
/* ' Handles the "Drop" message. */
/* ' */
/* ' @param    userIndex The index of the user sending the message. */

void HandleDrop(int UserIndex) {
	/* '*************************************************** */
	/* 'Author: Juan Martín Sotuyo Dodero (Maraxus) */
	/* 'Last Modification: 07/25/09 */
	/* '07/25/09: Marco - Agregué un checkeo para patear a los usuarios que tiran items mientras comercian. */
	/* '*************************************************** */
	if (UserList[UserIndex].sockctx->incomingData->length() < 4) {
		throw insufficient_data_error();
		return;
	}

	int Slot;
	int Amount;

	/* 'Remove packet ID */
	UserList[UserIndex].sockctx->incomingData->ReadByte();

	Slot = UserList[UserIndex].sockctx->incomingData->ReadByte();
	Amount = UserList[UserIndex].sockctx->incomingData->ReadInteger();

	/* 'low rank admins can't drop item. Neither can the dead nor those sailing. */
	if (UserList[UserIndex].flags.Navegando == 1 || UserList[UserIndex].flags.Muerto == 1
			|| (UserTieneAlgunPrivilegios(UserIndex, PlayerType_Consejero)
					&& !UserTieneAlgunPrivilegios(UserIndex, PlayerType_RoleMaster))) {
		return;
	}

	/* 'If the user is trading, he can't drop items => He's cheating, we kick him. */
	if (UserList[UserIndex].flags.Comerciando) {
		return;
	}

	/* 'Are we dropping gold or other items?? */
	if (Slot == FLAGORO) {
		/* 'Don't drop too much gold */
		if (Amount > 10000) {
			return;
		}

		TirarOro(Amount, UserIndex);

		WriteUpdateGold(UserIndex);
	} else {
		/* 'Only drop valid slots */
		if (Slot <= MAX_INVENTORY_SLOTS && Slot > 0) {
			if (UserList[UserIndex].Invent.Object[Slot].ObjIndex == 0) {
				return;
			}

			DropObj(UserIndex, Slot, Amount, UserList[UserIndex].Pos.Map, UserList[UserIndex].Pos.X,
					UserList[UserIndex].Pos.Y, true);
		}
	}
}

/* '' */
/* ' Handles the "CastSpell" message. */
/* ' */
/* ' @param    userIndex The index of the user sending the message. */

void HandleCastSpell(int UserIndex) {
	/* '*************************************************** */
	/* 'Author: Juan Martín Sotuyo Dodero (Maraxus) */
	/* 'Last Modification: 05/17/06 */
	/* '13/11/2009: ZaMa - Ahora los npcs pueden atacar al usuario si quizo castear un hechizo */
	/* '*************************************************** */
	if (UserList[UserIndex].sockctx->incomingData->length() < 2) {
		throw insufficient_data_error();
		return;
	}

	/* 'Remove packet ID */
	UserList[UserIndex].sockctx->incomingData->ReadByte();

	int Spell;

	Spell = UserList[UserIndex].sockctx->incomingData->ReadByte();

	if (UserList[UserIndex].flags.Muerto == 1) {
		WriteConsoleMsg(UserIndex, "¡¡Estás muerto!!", FontTypeNames_FONTTYPE_INFO);
		return;
	}

	/* 'Now you can be atacked */
	UserList[UserIndex].flags.NoPuedeSerAtacado = false;

	if (Spell < 1) {
		UserList[UserIndex].flags.Hechizo = 0;
		return;
	} else if (Spell > MAXUSERHECHIZOS) {
		UserList[UserIndex].flags.Hechizo = 0;
		return;
	}

	UserList[UserIndex].flags.Hechizo = UserList[UserIndex].Stats.UserHechizos[Spell];
}

/* '' */
/* ' Handles the "LeftClick" message. */
/* ' */
/* ' @param    userIndex The index of the user sending the message. */

void HandleLeftClick(int UserIndex) {
	/* '*************************************************** */
	/* 'Author: Juan Martín Sotuyo Dodero (Maraxus) */
	/* 'Last Modification: 05/17/06 */
	/* ' */
	/* '*************************************************** */
	if (UserList[UserIndex].sockctx->incomingData->length() < 3) {
		throw insufficient_data_error();
		return;
	}

	/* 'Remove packet ID */
	UserList[UserIndex].sockctx->incomingData->ReadByte();

	int X;
	int Y;

	X = UserList[UserIndex].sockctx->incomingData->ReadByte();
	Y = UserList[UserIndex].sockctx->incomingData->ReadByte();

	LookatTile(UserIndex, UserList[UserIndex].Pos.Map, X, Y);
}

/* '' */
/* ' Handles the "DoubleClick" message. */
/* ' */
/* ' @param    userIndex The index of the user sending the message. */

void HandleDoubleClick(int UserIndex) {
	/* '*************************************************** */
	/* 'Author: Juan Martín Sotuyo Dodero (Maraxus) */
	/* 'Last Modification: 05/17/06 */
	/* ' */
	/* '*************************************************** */
	if (UserList[UserIndex].sockctx->incomingData->length() < 3) {
		throw insufficient_data_error();
		return;
	}

	/* 'Remove packet ID */
	UserList[UserIndex].sockctx->incomingData->ReadByte();

	int X;
	int Y;

	X = UserList[UserIndex].sockctx->incomingData->ReadByte();
	Y = UserList[UserIndex].sockctx->incomingData->ReadByte();

	Accion(UserIndex, UserList[UserIndex].Pos.Map, X, Y);
}

/* '' */
/* ' Handles the "Work" message. */
/* ' */
/* ' @param    userIndex The index of the user sending the message. */

void HandleWork(int UserIndex) {
	/* '*************************************************** */
	/* 'Author: Juan Martín Sotuyo Dodero (Maraxus) */
	/* 'Last Modification: 13/01/2010 (ZaMa) */
	/* '13/01/2010: ZaMa - El pirata se puede ocultar en barca */
	/* '*************************************************** */
	if (UserList[UserIndex].sockctx->incomingData->length() < 2) {
		throw insufficient_data_error();
		return;
	}

	/* 'Remove packet ID */
	UserList[UserIndex].sockctx->incomingData->ReadByte();

	eSkill Skill;

	Skill = static_cast<eSkill> (UserList[UserIndex].sockctx->incomingData->ReadByte());

	if (UserList[UserIndex].flags.Muerto == 1) {
		return;
	}

	/* 'If exiting, cancel */
	CancelExit(UserIndex);

	switch (Skill) {
	case eSkill_Robar:
	case eSkill_Magia:
	case eSkill_Domar:
		WriteMultiMessage(UserIndex, eMessages_WorkRequestTarget, Skill);
		break;

	case eSkill_Ocultarse:

		/* ' Verifico si se peude ocultar en este mapa */
		if (MapInfo[UserList[UserIndex].Pos.Map].OcultarSinEfecto == 1) {
			WriteConsoleMsg(UserIndex, "¡Ocultarse no funciona aquí!", FontTypeNames_FONTTYPE_INFO);
			return;
		}

		if (UserList[UserIndex].flags.EnConsulta) {
			WriteConsoleMsg(UserIndex, "No puedes ocultarte si estás en consulta.",
					FontTypeNames_FONTTYPE_INFO);
			return;
		}

		if (UserList[UserIndex].flags.Navegando == 1) {
			if (UserList[UserIndex].clase != eClass_Pirat) {
				/* '[CDT 17-02-2004] */
				if (UserList[UserIndex].flags.UltimoMensaje != 3) {
					WriteConsoleMsg(UserIndex, "No puedes ocultarte si estás navegando.",
							FontTypeNames_FONTTYPE_INFO);
					UserList[UserIndex].flags.UltimoMensaje = 3;
				}
				/* '[/CDT] */
				return;
			}
		}

		if (UserList[UserIndex].flags.Oculto == 1) {
			/* '[CDT 17-02-2004] */
			if (UserList[UserIndex].flags.UltimoMensaje != 2) {
				WriteConsoleMsg(UserIndex, "Ya estás oculto.", FontTypeNames_FONTTYPE_INFO);
				UserList[UserIndex].flags.UltimoMensaje = 2;
			}
			/* '[/CDT] */
			return;
		}

		DoOcultarse(UserIndex);
		break;

	default:
		break;
	}

}

/* '' */
/* ' Handles the "InitCrafting" message. */
/* ' */
/* ' @param    userIndex The index of the user sending the message. */

void HandleInitCrafting(int UserIndex) {
	/* '*************************************************** */
	/* 'Author: ZaMa */
	/* 'Last Modification: 29/01/2010 */
	/* ' */
	/* '*************************************************** */
	int TotalItems;
	int ItemsPorCiclo;

	/* 'Remove packet ID */
	UserList[UserIndex].sockctx->incomingData->ReadByte();

	TotalItems = UserList[UserIndex].sockctx->incomingData->ReadLong();
	ItemsPorCiclo = UserList[UserIndex].sockctx->incomingData->ReadInteger();

	if (TotalItems > 0) {

		UserList[UserIndex].Construir.Cantidad = TotalItems;
		UserList[UserIndex].Construir.PorCiclo = MinimoInt(MaxItemsConstruibles(UserIndex), ItemsPorCiclo);
	}
}

/* '' */
/* ' Handles the "UseSpellMacro" message. */
/* ' */
/* ' @param    userIndex The index of the user sending the message. */

void HandleUseSpellMacro(int UserIndex) {
	/* '*************************************************** */
	/* 'Author: Juan Martín Sotuyo Dodero (Maraxus) */
	/* 'Last Modification: 05/17/06 */
	/* ' */
	/* '*************************************************** */
	/* 'Remove packet ID */
	UserList[UserIndex].sockctx->incomingData->ReadByte();

	SendData(SendTarget_ToAdmins, UserIndex,
			PrepareMessageConsoleMsg(UserList[UserIndex].Name + " fue expulsado por Anti-macro de hechizos.",
					FontTypeNames_FONTTYPE_VENENO));
	WriteErrorMsg(UserIndex,
			"Has sido expulsado por usar macro de hechizos. Recomendamos leer el reglamento sobre el tema macros.");
	FlushBuffer(UserIndex);
	CloseSocket(UserIndex);
}

/* '' */
/* ' Handles the "UseItem" message. */
/* ' */
/* ' @param    userIndex The index of the user sending the message. */

void HandleUseItem(int UserIndex) {
	/* '*************************************************** */
	/* 'Author: Juan Martín Sotuyo Dodero (Maraxus) */
	/* 'Last Modification: 05/17/06 */
	/* ' */
	/* '*************************************************** */
	if (UserList[UserIndex].sockctx->incomingData->length() < 2) {
		throw insufficient_data_error();
		return;
	}

	/* 'Remove packet ID */
	UserList[UserIndex].sockctx->incomingData->ReadByte();

	int Slot;

	Slot = UserList[UserIndex].sockctx->incomingData->ReadByte();

	if (Slot <= UserList[UserIndex].CurrentInventorySlots && Slot > 0) {
		if (UserList[UserIndex].Invent.Object[Slot].ObjIndex == 0) {
			return;
		}
	}

	if (UserList[UserIndex].flags.Meditando) {
		/* 'The error message should have been provided by the client. */
		return;
	}

	/* # IF SeguridadAlkon THEN */
	/* # END IF */

	UseInvItem(UserIndex, Slot);
}

/* '' */
/* ' Handles the "CraftBlacksmith" message. */
/* ' */
/* ' @param    userIndex The index of the user sending the message. */

void HandleCraftBlacksmith(int UserIndex) {
	/* '*************************************************** */
	/* 'Author: Juan Martín Sotuyo Dodero (Maraxus) */
	/* 'Last Modification: 05/17/06 */
	/* ' */
	/* '*************************************************** */
	if (UserList[UserIndex].sockctx->incomingData->length() < 3) {
		throw insufficient_data_error();
		return;
	}

	/* 'Remove packet ID */
	UserList[UserIndex].sockctx->incomingData->ReadByte();

	int Item;

	Item = UserList[UserIndex].sockctx->incomingData->ReadInteger();

	if (Item < 1) {
		return;
	}

	if (ObjData[Item].SkHerreria == 0) {
		return;
	}

	if (!IntervaloPermiteTrabajar(UserIndex)) {
		return;
	}
	HerreroConstruirItem(UserIndex, Item);
}

/* '' */
/* ' Handles the "CraftCarpenter" message. */
/* ' */
/* ' @param    userIndex The index of the user sending the message. */

void HandleCraftCarpenter(int UserIndex) {
	/* '*************************************************** */
	/* 'Author: Juan Martín Sotuyo Dodero (Maraxus) */
	/* 'Last Modification: 05/17/06 */
	/* ' */
	/* '*************************************************** */
	if (UserList[UserIndex].sockctx->incomingData->length() < 3) {
		throw insufficient_data_error();
		return;
	}

	/* 'Remove packet ID */
	UserList[UserIndex].sockctx->incomingData->ReadByte();

	int Item;

	Item = UserList[UserIndex].sockctx->incomingData->ReadInteger();

	if (Item < 1) {
		return;
	}

	if (ObjData[Item].SkCarpinteria == 0) {
		return;
	}

	if (!IntervaloPermiteTrabajar(UserIndex)) {
		return;
	}
	CarpinteroConstruirItem(UserIndex, Item);
}

/* '' */
/* ' Handles the "WorkLeftClick" message. */
/* ' */
/* ' @param    userIndex The index of the user sending the message. */

void HandleWorkLeftClick(int UserIndex) {
	/* '*************************************************** */
	/* 'Author: Juan Martín Sotuyo Dodero (Maraxus) */
	/* 'Last Modification: 14/01/2010 (ZaMa) */
	/* '16/11/2009: ZaMa - Agregada la posibilidad de extraer madera elfica. */
	/* '12/01/2010: ZaMa - Ahora se admiten armas arrojadizas (proyectiles sin municiones). */
	/* '14/01/2010: ZaMa - Ya no se pierden municiones al atacar npcs con dueno. */
	/* '*************************************************** */
	if (UserList[UserIndex].sockctx->incomingData->length() < 4) {
		throw insufficient_data_error();
		return;
	}

	/* 'Remove packet ID */
	UserList[UserIndex].sockctx->incomingData->ReadByte();

	int X;
	int Y;
	eSkill Skill;
	int DummyInt;
	/* 'Target user */
	int tU;
	/* 'Target NPC */
	int tN;

	int WeaponIndex;

	X = UserList[UserIndex].sockctx->incomingData->ReadByte();
	Y = UserList[UserIndex].sockctx->incomingData->ReadByte();

	Skill = static_cast<eSkill> (UserList[UserIndex].sockctx->incomingData->ReadByte());

	if (UserList[UserIndex].flags.Muerto == 1 || UserList[UserIndex].flags.Descansar
			|| UserList[UserIndex].flags.Meditando || !InMapBounds(UserList[UserIndex].Pos.Map, X, Y)) {
		return;
	}

	if (!InRangoVision(UserIndex, X, Y)) {
		WritePosUpdate(UserIndex);
		return;
	}

	/* 'If exiting, cancel */
	CancelExit(UserIndex);

	switch (Skill) {
	case eSkill_Proyectiles:

		/* 'Check attack interval */
		if (!IntervaloPermiteAtacar(UserIndex, false)) {
			return;
		}
		/* 'Check Magic interval */
		if (!IntervaloPermiteLanzarSpell(UserIndex, false)) {
			return;
		}
		/* 'Check bow's interval */
		if (!IntervaloPermiteUsarArcos(UserIndex)) {
			return;
		}

		LanzarProyectil(UserIndex, X, Y);

		break;

	case eSkill_Magia:
		/* 'Check the map allows spells to be casted. */
		if (MapInfo[UserList[UserIndex].Pos.Map].MagiaSinEfecto > 0) {
			WriteConsoleMsg(UserIndex, "Una fuerza oscura te impide canalizar tu energía.",
					FontTypeNames_FONTTYPE_FIGHT);
			return;
		}

		/* 'Target whatever is in that tile */
		LookatTile(UserIndex, UserList[UserIndex].Pos.Map, X, Y);

		/* 'If it's outside range log it and exit */
		if (vb6::Abs(UserList[UserIndex].Pos.X - X) > RANGO_VISION_X
				|| vb6::Abs(UserList[UserIndex].Pos.Y - Y) > RANGO_VISION_Y) {
			LogCheating(
					"Ataque fuera de rango de " + UserList[UserIndex].Name + "(" + vb6::CStr(UserList[UserIndex].Pos.Map)
							+ "/" + vb6::CStr(UserList[UserIndex].Pos.X) + "/" + vb6::CStr(UserList[UserIndex].Pos.Y) + ") ip: "
							+ UserList[UserIndex].ip + " a la posición (" + vb6::CStr(UserList[UserIndex].Pos.Map) + "/"
							+ vb6::CStr(X) + "/" + vb6::CStr(Y) + ")");
			return;
		}

		/* 'Check bow's interval */
		if (!IntervaloPermiteUsarArcos(UserIndex, false)) {
			return;
		}

		/* 'Check Spell-Hit interval */
		if (!IntervaloPermiteGolpeMagia(UserIndex)) {
			/* 'Check Magic interval */
			if (!IntervaloPermiteLanzarSpell(UserIndex)) {
				return;
			}
		}

		/* 'Check intervals and cast */
		if (UserList[UserIndex].flags.Hechizo > 0) {
			LanzarHechizo(UserList[UserIndex].flags.Hechizo, UserIndex);
			UserList[UserIndex].flags.Hechizo = 0;
		} else {
			WriteConsoleMsg(UserIndex, "¡Primero selecciona el hechizo que quieres lanzar!",
					FontTypeNames_FONTTYPE_INFO);
		}

		break;

	case eSkill_Pesca:
		WeaponIndex = UserList[UserIndex].Invent.WeaponEqpObjIndex;
		if (WeaponIndex == 0) {
			return;
		}

		/* 'Check interval */
		if (!IntervaloPermiteTrabajar(UserIndex)) {
			return;
		}

		/* 'Basado en la idea de Barrin */
		/* 'Comentario por Barrin: jah, "basado", caradura ! ^^ */
		if (MapData[UserList[UserIndex].Pos.Map][UserList[UserIndex].Pos.X][UserList[UserIndex].Pos.Y].trigger
				== 1) {
			WriteConsoleMsg(UserIndex, "No puedes pescar desde donde te encuentras.",
					FontTypeNames_FONTTYPE_INFO);
			return;
		}

		if (HayAgua(UserList[UserIndex].Pos.Map, X, Y)) {
			if (WeaponIndex == CANA_PESCA || WeaponIndex == CANA_PESCA_NEWBIE) {
				DoPescar(UserIndex);

			} else if (WeaponIndex == RED_PESCA) {
				DummyInt = MapData[UserList[UserIndex].Pos.Map][X][Y].ObjInfo.ObjIndex;

				if (DummyInt == 0) {
					WriteConsoleMsg(UserIndex, "No hay un yacimiento de peces donde pescar.",
							FontTypeNames_FONTTYPE_INFO);
					return;
				}

				if (vb6::Abs(UserList[UserIndex].Pos.X - X) + vb6::Abs(UserList[UserIndex].Pos.Y - Y) > 2) {
					WriteConsoleMsg(UserIndex, "Estás demasiado lejos para pescar.",
							FontTypeNames_FONTTYPE_INFO);
					return;
				}

				if (UserList[UserIndex].Pos.X == X && UserList[UserIndex].Pos.Y == Y) {
					WriteConsoleMsg(UserIndex, "No puedes pescar desde allí.", FontTypeNames_FONTTYPE_INFO);
					return;
				}

				/* '¿Hay un arbol normal donde clickeo? */
				if (ObjData[DummyInt].OBJType == eOBJType_otYacimientoPez) {
					DoPescarRed(UserIndex);
				} else {
					WriteConsoleMsg(UserIndex, "No hay un yacimiento de peces donde pescar.",
							FontTypeNames_FONTTYPE_INFO);
					return;
				}

				break;
			} else {
				/* 'Invalid item! */
			}

			/* 'Play sound! */
			SendData(SendTarget_ToPCArea, UserIndex,
					PrepareMessagePlayWave(SND_PESCAR, UserList[UserIndex].Pos.X, UserList[UserIndex].Pos.Y));
		} else {
			WriteConsoleMsg(UserIndex, "No hay agua donde pescar. Busca un lago, río o mar.",
					FontTypeNames_FONTTYPE_INFO);
		}
		break;

	case eSkill_Robar:
		/* 'Does the map allow us to steal here? */
		if (MapInfo[UserList[UserIndex].Pos.Map].Pk) {

			/* 'Check interval */
			if (!IntervaloPermiteTrabajar(UserIndex)) {
				return;
			}

			/* 'Target whatever is in that tile */
			LookatTile(UserIndex, UserList[UserIndex].Pos.Map, X, Y);

			tU = UserList[UserIndex].flags.TargetUser;

			if (tU > 0 && tU != UserIndex) {
				/* 'Can't steal administrative players */
				if (UserTieneAlgunPrivilegios(tU, PlayerType_User)) {
					if (UserList[tU].flags.Muerto == 0) {
						if (vb6::Abs(UserList[UserIndex].Pos.X - X) + vb6::Abs(UserList[UserIndex].Pos.Y - Y)
								> 2) {
							WriteConsoleMsg(UserIndex, "Estás demasiado lejos.", FontTypeNames_FONTTYPE_INFO);
							return;
						}

						/* '17/09/02 */
						/* 'Check the trigger */
						if (MapData[UserList[tU].Pos.Map][X][Y].trigger == eTrigger_ZONASEGURA) {
							WriteConsoleMsg(UserIndex, "No puedes robar aquí.",
									FontTypeNames_FONTTYPE_WARNING);
							return;
						}

						if (MapData[UserList[UserIndex].Pos.Map][UserList[UserIndex].Pos.X][UserList[UserIndex].Pos.Y].trigger
								== eTrigger_ZONASEGURA) {
							WriteConsoleMsg(UserIndex, "No puedes robar aquí.",
									FontTypeNames_FONTTYPE_WARNING);
							return;
						}

						DoRobar(UserIndex, tU);
					}
				}
			} else {
				WriteConsoleMsg(UserIndex, "¡No hay a quien robarle!", FontTypeNames_FONTTYPE_INFO);
			}
		} else {
			WriteConsoleMsg(UserIndex, "¡No puedes robar en zonas seguras!", FontTypeNames_FONTTYPE_INFO);
		}

		break;

	case eSkill_Talar:
		/* 'Check interval */
		if (!IntervaloPermiteTrabajar(UserIndex)) {
			return;
		}

		WeaponIndex = UserList[UserIndex].Invent.WeaponEqpObjIndex;

		if (WeaponIndex == 0) {
			WriteConsoleMsg(UserIndex, "Deberías equiparte el hacha.", FontTypeNames_FONTTYPE_INFO);
			return;
		}

		if (WeaponIndex != HACHA_LENADOR && WeaponIndex != HACHA_LENA_ELFICA
				&& WeaponIndex != HACHA_LENADOR_NEWBIE) {
			/* ' Podemos llegar acá si el user equipó el anillo dsp de la U y antes del click */
			return;
		}

		DummyInt = MapData[UserList[UserIndex].Pos.Map][X][Y].ObjInfo.ObjIndex;

		if (DummyInt > 0) {
			if (vb6::Abs(UserList[UserIndex].Pos.X - X) + vb6::Abs(UserList[UserIndex].Pos.Y - Y) > 2) {
				WriteConsoleMsg(UserIndex, "Estás demasiado lejos.", FontTypeNames_FONTTYPE_INFO);
				return;
			}

			/* 'Barrin 29/9/03 */
			if (UserList[UserIndex].Pos.X == X && UserList[UserIndex].Pos.Y == Y) {
				WriteConsoleMsg(UserIndex, "No puedes talar desde allí.", FontTypeNames_FONTTYPE_INFO);
				return;
			}

			/* '¿Hay un arbol normal donde clickeo? */
			if (ObjData[DummyInt].OBJType == eOBJType_otArboles) {
				if (WeaponIndex == HACHA_LENADOR || WeaponIndex == HACHA_LENADOR_NEWBIE) {
					SendData(SendTarget_ToPCArea, UserIndex,
							PrepareMessagePlayWave(SND_TALAR, UserList[UserIndex].Pos.X,
									UserList[UserIndex].Pos.Y));
					DoTalar(UserIndex);
				} else {
					WriteConsoleMsg(UserIndex, "No puedes extraer lena de éste árbol con éste hacha.",
							FontTypeNames_FONTTYPE_INFO);
				}

				/* ' Arbol Elfico? */
			} else if (ObjData[DummyInt].OBJType == eOBJType_otArbolElfico) {

				if (WeaponIndex == HACHA_LENA_ELFICA) {
					SendData(SendTarget_ToPCArea, UserIndex,
							PrepareMessagePlayWave(SND_TALAR, UserList[UserIndex].Pos.X,
									UserList[UserIndex].Pos.Y));
					DoTalar(UserIndex, true);
				} else {
					WriteConsoleMsg(UserIndex, "El hacha utilizado no es suficientemente poderosa.",
							FontTypeNames_FONTTYPE_INFO);
				}
			}
		} else {
			WriteConsoleMsg(UserIndex, "No hay ningún árbol ahí.", FontTypeNames_FONTTYPE_INFO);
		}

		break;

	case eSkill_Mineria:
		if (!IntervaloPermiteTrabajar(UserIndex)) {
			return;
		}

		WeaponIndex = UserList[UserIndex].Invent.WeaponEqpObjIndex;

		if (WeaponIndex == 0) {
			return;
		}

		if (WeaponIndex != PIQUETE_MINERO && WeaponIndex != PIQUETE_MINERO_NEWBIE) {
			/* ' Podemos llegar acá si el user equipó el anillo dsp de la U y antes del click */
			return;
		}

		/* 'Target whatever is in the tile */
		LookatTile(UserIndex, UserList[UserIndex].Pos.Map, X, Y);

		DummyInt = MapData[UserList[UserIndex].Pos.Map][X][Y].ObjInfo.ObjIndex;

		if (DummyInt > 0) {
			/* 'Check distance */
			if (vb6::Abs(UserList[UserIndex].Pos.X - X) + vb6::Abs(UserList[UserIndex].Pos.Y - Y) > 2) {
				WriteConsoleMsg(UserIndex, "Estás demasiado lejos.", FontTypeNames_FONTTYPE_INFO);
				return;
			}

			/* '¿Hay un yacimiento donde clickeo? */
			if (ObjData[DummyInt].OBJType == eOBJType_otYacimiento) {
				DoMineria(UserIndex);
			} else {
				WriteConsoleMsg(UserIndex, "Ahí no hay ningún yacimiento.", FontTypeNames_FONTTYPE_INFO);
			}
		} else {
			WriteConsoleMsg(UserIndex, "Ahí no hay ningún yacimiento.", FontTypeNames_FONTTYPE_INFO);
		}

		break;

	case eSkill_Domar:
		/* 'Modificado 25/11/02 */
		/* 'Optimizado y solucionado el bug de la doma de */
		/* 'criaturas hostiles. */

		/* 'Target whatever is that tile */
		LookatTile(UserIndex, UserList[UserIndex].Pos.Map, X, Y);
		tN = UserList[UserIndex].flags.TargetNPC;

		if (tN > 0) {
			if (Npclist[tN].flags.Domable > 0) {
				if (vb6::Abs(UserList[UserIndex].Pos.X - X) + vb6::Abs(UserList[UserIndex].Pos.Y - Y) > 2) {
					WriteConsoleMsg(UserIndex, "Estás demasiado lejos.", FontTypeNames_FONTTYPE_INFO);
					return;
				}

				if (vb6::LenB(Npclist[tN].flags.AttackedBy) != 0) {
					WriteConsoleMsg(UserIndex,
							"No puedes domar una criatura que está luchando con un jugador.",
							FontTypeNames_FONTTYPE_INFO);
					return;
				}

				DoDomar(UserIndex, tN);
			} else {
				WriteConsoleMsg(UserIndex, "No puedes domar a esa criatura.", FontTypeNames_FONTTYPE_INFO);
			}
		} else {
			WriteConsoleMsg(UserIndex, "¡No hay ninguna criatura allí!", FontTypeNames_FONTTYPE_INFO);
		}

		/* 'UGLY!!! This is a constant, not a skill!! */
		break;

	case eSkill_FundirMetal:
		/* 'Check interval */
		if (!IntervaloPermiteTrabajar(UserIndex)) {
			return;
		}

		/* 'Check there is a proper item there */
		if (UserList[UserIndex].flags.TargetObj > 0) {
			if (ObjData[UserList[UserIndex].flags.TargetObj].OBJType == eOBJType_otFragua) {
				/* 'Validate other items */
				if (UserList[UserIndex].flags.TargetObjInvSlot < 1
						|| UserList[UserIndex].flags.TargetObjInvSlot
								> UserList[UserIndex].CurrentInventorySlots) {
					return;
				}

				/* ''chequeamos que no se zarpe duplicando oro */
				if (UserList[UserIndex].Invent.Object[UserList[UserIndex].flags.TargetObjInvSlot].ObjIndex
						!= UserList[UserIndex].flags.TargetObjInvIndex) {
					if (UserList[UserIndex].Invent.Object[UserList[UserIndex].flags.TargetObjInvSlot].ObjIndex
							== 0
							|| UserList[UserIndex].Invent.Object[UserList[UserIndex].flags.TargetObjInvSlot].Amount
									== 0) {
						WriteConsoleMsg(UserIndex, "No tienes más minerales.", FontTypeNames_FONTTYPE_INFO);
						return;
					}

					/* ''FUISTE */
					WriteErrorMsg(UserIndex, "Has sido expulsado por el sistema anti cheats.");
					FlushBuffer(UserIndex);
					CloseSocket(UserIndex);
					return;
				}
				if (ObjData[UserList[UserIndex].flags.TargetObjInvIndex].OBJType == eOBJType_otMinerales) {
					FundirMineral(UserIndex);
				} else if (ObjData[UserList[UserIndex].flags.TargetObjInvIndex].OBJType
						== eOBJType_otWeapon) {
					FundirArmas(UserIndex);
				}
			} else {
				WriteConsoleMsg(UserIndex, "Ahí no hay ninguna fragua.", FontTypeNames_FONTTYPE_INFO);
			}
		} else {
			WriteConsoleMsg(UserIndex, "Ahí no hay ninguna fragua.", FontTypeNames_FONTTYPE_INFO);
		}

		break;

	case eSkill_Herreria:
		/* 'Target wehatever is in that tile */
		LookatTile(UserIndex, UserList[UserIndex].Pos.Map, X, Y);

		if (UserList[UserIndex].flags.TargetObj > 0) {
			if (ObjData[UserList[UserIndex].flags.TargetObj].OBJType == eOBJType_otYunque) {
				EnivarArmasConstruibles(UserIndex);
				EnivarArmadurasConstruibles(UserIndex);
				WriteShowBlacksmithForm(UserIndex);
			} else {
				WriteConsoleMsg(UserIndex, "Ahí no hay ningún yunque.", FontTypeNames_FONTTYPE_INFO);
			}
		} else {
			WriteConsoleMsg(UserIndex, "Ahí no hay ningún yunque.", FontTypeNames_FONTTYPE_INFO);
		}
		break;

	default:
		break;
	}
}

/* '' */
/* ' Handles the "CreateNewGuild" message. */
/* ' */
/* ' @param    userIndex The index of the user sending the message. */

void HandleCreateNewGuild(int UserIndex) {
	/* '*************************************************** */
	/* 'Author: Juan Martín Sotuyo Dodero (Maraxus) */
	/* 'Last Modification: 05/11/09 */
	/* '05/11/09: Pato - Ahora se quitan los espacios del principio y del fin del nombre del clan */
	/* '*************************************************** */
	if (UserList[UserIndex].sockctx->incomingData->length() < 9) {
		throw insufficient_data_error();
		return;
	}

	/* 'This packet contains strings, make a copy of the data to prevent losses if it's not complete yet... */
	clsByteQueue* buffer = UserList[UserIndex].sockctx->incomingData.get();


	/* 'Remove packet ID */
	buffer->ReadByte();

	std::string desc;
	std::string GuildName;
	std::string site;
	std::vector<std::string> codex;
	std::string errorStr;

	desc = buffer->ReadUnicodeString();
	GuildName = vb6::Trim(buffer->ReadUnicodeString());
	site = buffer->ReadUnicodeString();
	codex = vb6::Split(buffer->ReadUnicodeString(), SEPARATOR);

	if (CrearNuevoClan(UserIndex, desc, GuildName, site, codex, UserList[UserIndex].FundandoGuildAlineacion,
			errorStr)) {
		SendData(SendTarget_ToAll, UserIndex,
				PrepareMessageConsoleMsg(
						UserList[UserIndex].Name + " fundó el clan " + GuildName + " de alineación "
								+ GuildAlignment(UserList[UserIndex].GuildIndex) + ".",
						FontTypeNames_FONTTYPE_GUILD));
		SendData(SendTarget_ToAll, 0, PrepareMessagePlayWave(44, NO_3D_SOUND, NO_3D_SOUND));

		/* 'Update tag */
		RefreshCharStatus(UserIndex);
	} else {
		WriteConsoleMsg(UserIndex, errorStr, FontTypeNames_FONTTYPE_GUILD);
	}

	/* 'If we got here then packet is complete, copy data back to original queue */

}

/* '' */
/* ' Handles the "SpellInfo" message. */
/* ' */
/* ' @param    userIndex The index of the user sending the message. */

void HandleSpellInfo(int UserIndex) {
	/* '*************************************************** */
	/* 'Author: Juan Martín Sotuyo Dodero (Maraxus) */
	/* 'Last Modification: 05/17/06 */
	/* ' */
	/* '*************************************************** */
	if (UserList[UserIndex].sockctx->incomingData->length() < 2) {
		throw insufficient_data_error();
		return;
	}

	/* 'Remove packet ID */
	UserList[UserIndex].sockctx->incomingData->ReadByte();

	int spellSlot;
	int Spell;

	spellSlot = UserList[UserIndex].sockctx->incomingData->ReadByte();

	/* 'Validate slot */
	if (spellSlot < 1 || spellSlot > MAXUSERHECHIZOS) {
		WriteConsoleMsg(UserIndex, "¡Primero selecciona el hechizo!", FontTypeNames_FONTTYPE_INFO);
		return;
	}

	/* 'Validate spell in the slot */
	Spell = UserList[UserIndex].Stats.UserHechizos[spellSlot];
	if (Spell > 0 && Spell < NumeroHechizos + 1) {
		/* 'Send information */
		WriteConsoleMsg(UserIndex,
				vb6::CStr("%%%%%%%%%%%% INFO DEL HECHIZO %%%%%%%%%%%%") + vbCrLf + "Nombre:" + Hechizos[Spell].Nombre
						+ vbCrLf + "Descripción:" + Hechizos[Spell].desc + vbCrLf + "Skill requerido: "
						+ vb6::CStr(Hechizos[Spell].MinSkill) + " de magia." + vbCrLf + "Maná necesario: "
						+ vb6::CStr(Hechizos[Spell].ManaRequerido) + vbCrLf + "Energía necesaria: "
						+ vb6::CStr(Hechizos[Spell].StaRequerido) + vbCrLf + "%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%",
				FontTypeNames_FONTTYPE_INFO);
	}
}

/* '' */
/* ' Handles the "EquipItem" message. */
/* ' */
/* ' @param    userIndex The index of the user sending the message. */

void HandleEquipItem(int UserIndex) {
	/* '*************************************************** */
	/* 'Author: Juan Martín Sotuyo Dodero (Maraxus) */
	/* 'Last Modification: 05/17/06 */
	/* ' */
	/* '*************************************************** */
	if (UserList[UserIndex].sockctx->incomingData->length() < 2) {
		throw insufficient_data_error();
		return;
	}

	/* 'Remove packet ID */
	UserList[UserIndex].sockctx->incomingData->ReadByte();

	int itemSlot;

	itemSlot = UserList[UserIndex].sockctx->incomingData->ReadByte();

	/* 'Dead users can't equip items */
	if (UserList[UserIndex].flags.Muerto == 1) {
		return;
	}

	/* 'Validate item slot */
	if (itemSlot > UserList[UserIndex].CurrentInventorySlots || itemSlot < 1) {
		return;
	}

	if (UserList[UserIndex].Invent.Object[itemSlot].ObjIndex == 0) {
		return;
	}

	EquiparInvItem(UserIndex, itemSlot);
}

/* '' */
/* ' Handles the "ChangeHeading" message. */
/* ' */
/* ' @param    userIndex The index of the user sending the message. */

void HandleChangeHeading(int UserIndex) {
	/* '*************************************************** */
	/* 'Author: Juan Martín Sotuyo Dodero (Maraxus) */
	/* 'Last Modification: 06/28/2008 */
	/* 'Last Modified By: NicoNZ */
	/* ' 10/01/2008: Tavo - Se cancela la salida del juego si el user esta saliendo */
	/* ' 06/28/2008: NicoNZ - Sólo se puede cambiar si está inmovilizado. */
	/* '*************************************************** */
	if (UserList[UserIndex].sockctx->incomingData->length() < 2) {
		throw insufficient_data_error();
		return;
	}

	/* 'Remove packet ID */
	UserList[UserIndex].sockctx->incomingData->ReadByte();

	eHeading heading;
	int posX = 0;
	int posY = 0;

	heading = static_cast<eHeading> (UserList[UserIndex].sockctx->incomingData->ReadByte());

	if (UserList[UserIndex].flags.Paralizado == 1 && UserList[UserIndex].flags.Inmovilizado == 0) {
		switch (heading) {
		case eHeading_NORTH:
			posY = -1;
			break;

		case eHeading_EAST:
			posX = 1;
			break;

		case eHeading_SOUTH:
			posY = 1;
			break;

		case eHeading_WEST:
			posX = -1;
			break;

		default:
			break;
		}

		if (LegalPos(UserList[UserIndex].Pos.Map, UserList[UserIndex].Pos.X + posX,
				UserList[UserIndex].Pos.Y + posY, vb6::CBool(UserList[UserIndex].flags.Navegando),
				!vb6::CBool(UserList[UserIndex].flags.Navegando))) {
			return;
		}
	}

	/* 'Validate heading (VB won't say invalid cast if not a valid index like .Net languages would do... *sigh*) */
	if (heading > 0 && heading < 5) {
		UserList[UserIndex].Char.heading = heading;
		ChangeUserChar(UserIndex, UserList[UserIndex].Char.body, UserList[UserIndex].Char.Head,
				UserList[UserIndex].Char.heading, UserList[UserIndex].Char.WeaponAnim,
				UserList[UserIndex].Char.ShieldAnim, UserList[UserIndex].Char.CascoAnim);
	}
}

/* '' */
/* ' Handles the "ModifySkills" message. */
/* ' */
/* ' @param    userIndex The index of the user sending the message. */

void HandleModifySkills(int UserIndex) {
	/* '*************************************************** */
	/* 'Author: Juan Martín Sotuyo Dodero (Maraxus) */
	/* 'Last Modification: 11/19/09 */
	/* '11/19/09: Pato - Adapting to new skills system. */
	/* '*************************************************** */
	if (UserList[UserIndex].sockctx->incomingData->length() < 1 + NUMSKILLS) {
		throw insufficient_data_error();
		return;
	}

	/* 'Remove packet ID */
	UserList[UserIndex].sockctx->incomingData->ReadByte();

	int i;
	int Count = 0;
	vb6::array<int> points;

	points.redim(1, NUMSKILLS);

	/* 'Codigo para prevenir el hackeo de los skills */
	/* '<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< */
	for (i = (1); i <= (NUMSKILLS); i++) {
		points[i] = UserList[UserIndex].sockctx->incomingData->ReadByte();

		if (points[i] < 0) {
			LogHackAttemp(
					UserList[UserIndex].Name + " IP:" + UserList[UserIndex].ip
							+ " trató de hackear los skills.");
			UserList[UserIndex].Stats.SkillPts = 0;
			/* FIXME: Ban IP */
			CloseSocket(UserIndex);
			return;
		}

		Count = Count + points[i];
	}

	if (Count > UserList[UserIndex].Stats.SkillPts) {
		LogHackAttemp(
				UserList[UserIndex].Name + " IP:" + UserList[UserIndex].ip + " trató de hackear los skills.");
		/* FIXME: Ban IP */
		CloseSocket(UserIndex);
		return;
	}
	/* '<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< */

	UserList[UserIndex].Counters.AsignedSkills = MinimoInt(10,
			UserList[UserIndex].Counters.AsignedSkills + Count);

	for (i = (1); i <= (NUMSKILLS); i++) {
		if (points[i] > 0) {
			UserList[UserIndex].Stats.SkillPts = UserList[UserIndex].Stats.SkillPts - points[i];
			UserList[UserIndex].Stats.UserSkills[i] = UserList[UserIndex].Stats.UserSkills[i] + points[i];

			/* 'Client should prevent this, but just in case... */
			if (UserList[UserIndex].Stats.UserSkills[i] > 100) {
				/*UserList[UserIndex].Stats.SkillPts = UserList[UserIndex].Stats.SkillPts
						+ UserList[UserIndex].Stats.UserSkills[i] - 100;
				UserList[UserIndex].Stats.UserSkills[i] = 100;*/
				UserList[UserIndex].Stats.UserSkills[i] = 0;
				LogHackAttemp(
						UserList[UserIndex].Name + " IP:" + UserList[UserIndex].ip
								+ " trató de hackear los skills.");
				UserList[UserIndex].Stats.SkillPts = 0;
				/* FIXME: Ban IP */
				CloseSocket(UserIndex);
				return;
			}

			CheckEluSkill(UserIndex, i, true);
		}
	}
}

/* '' */
/* ' Handles the "Train" message. */
/* ' */
/* ' @param    userIndex The index of the user sending the message. */

void HandleTrain(int UserIndex) {
	/* '*************************************************** */
	/* 'Author: Juan Martín Sotuyo Dodero (Maraxus) */
	/* 'Last Modification: 05/17/06 */
	/* ' */
	/* '*************************************************** */
	if (UserList[UserIndex].sockctx->incomingData->length() < 2) {
		throw insufficient_data_error();
		return;
	}

	/* 'Remove packet ID */
	UserList[UserIndex].sockctx->incomingData->ReadByte();

	int SpawnedNpc;
	int PetIndex;

	PetIndex = UserList[UserIndex].sockctx->incomingData->ReadByte();

	if (UserList[UserIndex].flags.TargetNPC == 0) {
		return;
	}

	if (Npclist[UserList[UserIndex].flags.TargetNPC].NPCtype != eNPCType_Entrenador) {
		return;
	}

	if (Npclist[UserList[UserIndex].flags.TargetNPC].Mascotas < MAXMASCOTASENTRENADOR) {
		if (PetIndex > 0 && PetIndex < Npclist[UserList[UserIndex].flags.TargetNPC].NroCriaturas + 1) {
			/* 'Create the creature */
			SpawnedNpc = SpawnNpc(Npclist[UserList[UserIndex].flags.TargetNPC].Criaturas[PetIndex].NpcIndex,
					Npclist[UserList[UserIndex].flags.TargetNPC].Pos, true, false);

			if (SpawnedNpc > 0) {
				Npclist[SpawnedNpc].MaestroNpc = UserList[UserIndex].flags.TargetNPC;
				Npclist[UserList[UserIndex].flags.TargetNPC].Mascotas =
						Npclist[UserList[UserIndex].flags.TargetNPC].Mascotas + 1;
			}
		}
	} else {
		SendData(SendTarget_ToPCArea, UserIndex,
				PrepareMessageChatOverHead("No puedo traer más criaturas, mata las existentes.",
						Npclist[UserList[UserIndex].flags.TargetNPC].Char.CharIndex, 0x00ffffff));
	}
}

/* '' */
/* ' Handles the "CommerceBuy" message. */
/* ' */
/* ' @param    userIndex The index of the user sending the message. */

void HandleCommerceBuy(int UserIndex) {
	/* '*************************************************** */
	/* 'Author: Juan Martín Sotuyo Dodero (Maraxus) */
	/* 'Last Modification: 05/17/06 */
	/* ' */
	/* '*************************************************** */
	if (UserList[UserIndex].sockctx->incomingData->length() < 4) {
		throw insufficient_data_error();
		return;
	}

	/* 'Remove packet ID */
	UserList[UserIndex].sockctx->incomingData->ReadByte();

	int Slot;
	int Amount;

	Slot = UserList[UserIndex].sockctx->incomingData->ReadByte();
	Amount = UserList[UserIndex].sockctx->incomingData->ReadInteger();

	/* 'Dead people can't commerce... */
	if (UserList[UserIndex].flags.Muerto == 1) {
		WriteConsoleMsg(UserIndex, "¡¡Estás muerto!!", FontTypeNames_FONTTYPE_INFO);
		return;
	}

	/* '¿El target es un NPC valido? */
	if (UserList[UserIndex].flags.TargetNPC < 1) {
		return;
	}

	/* '¿El NPC puede comerciar? */
	if (Npclist[UserList[UserIndex].flags.TargetNPC].Comercia == 0) {
		SendData(SendTarget_ToPCArea, UserIndex,
				PrepareMessageChatOverHead("No tengo ningún interés en comerciar.",
						Npclist[UserList[UserIndex].flags.TargetNPC].Char.CharIndex, 0x00ffffff));
		return;
	}

	/* 'Only if in commerce mode.... */
	if (!UserList[UserIndex].flags.Comerciando) {
		WriteConsoleMsg(UserIndex, "No estás comerciando.", FontTypeNames_FONTTYPE_INFO);
		return;
	}

	/* 'User compra el item */
	Comercio(eModoComercio_Compra, UserIndex, UserList[UserIndex].flags.TargetNPC, Slot, Amount);
}

/* '' */
/* ' Handles the "BankExtractItem" message. */
/* ' */
/* ' @param    userIndex The index of the user sending the message. */

void HandleBankExtractItem(int UserIndex) {
	/* '*************************************************** */
	/* 'Author: Juan Martín Sotuyo Dodero (Maraxus) */
	/* 'Last Modification: 05/17/06 */
	/* ' */
	/* '*************************************************** */
	if (UserList[UserIndex].sockctx->incomingData->length() < 4) {
		throw insufficient_data_error();
		return;
	}

	/* 'Remove packet ID */
	UserList[UserIndex].sockctx->incomingData->ReadByte();

	int Slot;
	int Amount;

	Slot = UserList[UserIndex].sockctx->incomingData->ReadByte();
	Amount = UserList[UserIndex].sockctx->incomingData->ReadInteger();

	/* 'Dead people can't commerce */
	if (UserList[UserIndex].flags.Muerto == 1) {
		WriteConsoleMsg(UserIndex, "¡¡Estás muerto!!", FontTypeNames_FONTTYPE_INFO);
		return;
	}

	/* '¿El target es un NPC valido? */
	if (UserList[UserIndex].flags.TargetNPC < 1) {
		return;
	}

	/* '¿Es el banquero? */
	if (Npclist[UserList[UserIndex].flags.TargetNPC].NPCtype != eNPCType_Banquero) {
		return;
	}

	/* 'User retira el item del slot */
	UserRetiraItem(UserIndex, Slot, Amount);
}

/* '' */
/* ' Handles the "CommerceSell" message. */
/* ' */
/* ' @param    userIndex The index of the user sending the message. */

void HandleCommerceSell(int UserIndex) {
	/* '*************************************************** */
	/* 'Author: Juan Martín Sotuyo Dodero (Maraxus) */
	/* 'Last Modification: 05/17/06 */
	/* ' */
	/* '*************************************************** */
	if (UserList[UserIndex].sockctx->incomingData->length() < 4) {
		throw insufficient_data_error();
		return;
	}

	/* 'Remove packet ID */
	UserList[UserIndex].sockctx->incomingData->ReadByte();

	int Slot;
	int Amount;

	Slot = UserList[UserIndex].sockctx->incomingData->ReadByte();
	Amount = UserList[UserIndex].sockctx->incomingData->ReadInteger();

	/* 'Dead people can't commerce... */
	if (UserList[UserIndex].flags.Muerto == 1) {
		WriteConsoleMsg(UserIndex, "¡¡Estás muerto!!", FontTypeNames_FONTTYPE_INFO);
		return;
	}

	/* '¿El target es un NPC valido? */
	if (UserList[UserIndex].flags.TargetNPC < 1) {
		return;
	}

	/* '¿El NPC puede comerciar? */
	if (Npclist[UserList[UserIndex].flags.TargetNPC].Comercia == 0) {
		SendData(SendTarget_ToPCArea, UserIndex,
				PrepareMessageChatOverHead("No tengo ningún interés en comerciar.",
						Npclist[UserList[UserIndex].flags.TargetNPC].Char.CharIndex, 0x00ffffff));
		return;
	}

	/* 'User compra el item del slot */
	Comercio(eModoComercio_Venta, UserIndex, UserList[UserIndex].flags.TargetNPC, Slot, Amount);
}

/* '' */
/* ' Handles the "BankDeposit" message. */
/* ' */
/* ' @param    userIndex The index of the user sending the message. */

void HandleBankDeposit(int UserIndex) {
	/* '*************************************************** */
	/* 'Author: Juan Martín Sotuyo Dodero (Maraxus) */
	/* 'Last Modification: 05/17/06 */
	/* ' */
	/* '*************************************************** */
	if (UserList[UserIndex].sockctx->incomingData->length() < 4) {
		throw insufficient_data_error();
		return;
	}

	/* 'Remove packet ID */
	UserList[UserIndex].sockctx->incomingData->ReadByte();

	int Slot;
	int Amount;

	Slot = UserList[UserIndex].sockctx->incomingData->ReadByte();
	Amount = UserList[UserIndex].sockctx->incomingData->ReadInteger();

	/* 'Dead people can't commerce... */
	if (UserList[UserIndex].flags.Muerto == 1) {
		WriteConsoleMsg(UserIndex, "¡¡Estás muerto!!", FontTypeNames_FONTTYPE_INFO);
		return;
	}

	/* '¿El target es un NPC valido? */
	if (UserList[UserIndex].flags.TargetNPC < 1) {
		return;
	}

	/* '¿El NPC puede comerciar? */
	if (Npclist[UserList[UserIndex].flags.TargetNPC].NPCtype != eNPCType_Banquero) {
		return;
	}

	/* 'User deposita el item del slot rdata */
	UserDepositaItem(UserIndex, Slot, Amount);
}

/* '' */
/* ' Handles the "ForumPost" message. */
/* ' */
/* ' @param    userIndex The index of the user sending the message. */

void HandleForumPost(int UserIndex) {
	/* '*************************************************** */
	/* 'Author: Juan Martín Sotuyo Dodero (Maraxus) */
	/* 'Last Modification: 02/01/2010 */
	/* '02/01/2010: ZaMa - Implemento nuevo sistema de foros */
	/* '*************************************************** */
	if (UserList[UserIndex].sockctx->incomingData->length() < 6) {
		throw insufficient_data_error();
		return;
	}

	/* 'This packet contains strings, make a copy of the data to prevent losses if it's not complete yet... */
	clsByteQueue* buffer = UserList[UserIndex].sockctx->incomingData.get();


	/* 'Remove packet ID */
	buffer->ReadByte();

	eForumMsgType ForumMsgType;

	std::string File;
	std::string Title;
	std::string Post;
	int ForumIndex = 0;
	std::string postFile;
	int ForumType;

	ForumMsgType = static_cast<eForumMsgType>(buffer->ReadByte());

	Title = buffer->ReadUnicodeString();
	Post = buffer->ReadUnicodeString();

	if (UserList[UserIndex].flags.TargetObj > 0) {
		ForumType = ForumAlignment(ForumMsgType);

		switch (ForumType) {

		case eForumType_ieGeneral:
			ForumIndex = GetForumIndex(ObjData[UserList[UserIndex].flags.TargetObj].ForoID);

			break;

		case eForumType_ieREAL:
			ForumIndex = GetForumIndex(FORO_REAL_ID);

			break;

		case eForumType_ieCAOS:
			ForumIndex = GetForumIndex(FORO_CAOS_ID);

			break;
		}

		AddPost(ForumIndex, Post, UserList[UserIndex].Name, Title, EsAnuncio(ForumMsgType));
	}

	/* 'If we got here then packet is complete, copy data back to original queue */


}

/* '' */
/* ' Handles the "MoveSpell" message. */
/* ' */
/* ' @param    userIndex The index of the user sending the message. */

void HandleMoveSpell(int UserIndex) {
	/* '*************************************************** */
	/* 'Author: Juan Martín Sotuyo Dodero (Maraxus) */
	/* 'Last Modification: 05/17/06 */
	/* ' */
	/* '*************************************************** */
	if (UserList[UserIndex].sockctx->incomingData->length() < 3) {
		throw insufficient_data_error();
		return;
	}

	/* 'Remove packet ID */
	UserList[UserIndex].sockctx->incomingData->ReadByte();

	int dir;

	if (UserList[UserIndex].sockctx->incomingData->ReadBoolean()) {
		dir = 1;
	} else {
		dir = -1;
	}

	DesplazarHechizo(UserIndex, dir, UserList[UserIndex].sockctx->incomingData->ReadByte());
}

/* '' */
/* ' Handles the "MoveBank" message. */
/* ' */
/* ' @param    userIndex The index of the user sending the message. */

void HandleMoveBank(int UserIndex) {
	/* '*************************************************** */
	/* 'Author: Torres Patricio (Pato) */
	/* 'Last Modification: 06/14/09 */
	/* ' */
	/* '*************************************************** */
	if (UserList[UserIndex].sockctx->incomingData->length() < 3) {
		throw insufficient_data_error();
		return;
	}

	/* 'Remove packet ID */
	UserList[UserIndex].sockctx->incomingData->ReadByte();

	int dir;
	int Slot;
	struct Obj TempItem;

	if (UserList[UserIndex].sockctx->incomingData->ReadBoolean()) {
		dir = 1;
	} else {
		dir = -1;
	}

	Slot = UserList[UserIndex].sockctx->incomingData->ReadByte();

	TempItem.ObjIndex = UserList[UserIndex].BancoInvent.Object[Slot].ObjIndex;
	TempItem.Amount = UserList[UserIndex].BancoInvent.Object[Slot].Amount;

	/* 'Mover arriba */
	if (dir == 1) {
		UserList[UserIndex].BancoInvent.Object[Slot] = UserList[UserIndex].BancoInvent.Object[Slot - 1];
		UserList[UserIndex].BancoInvent.Object[Slot - 1].ObjIndex = TempItem.ObjIndex;
		UserList[UserIndex].BancoInvent.Object[Slot - 1].Amount = TempItem.Amount;
		/* 'mover abajo */
	} else {
		UserList[UserIndex].BancoInvent.Object[Slot] = UserList[UserIndex].BancoInvent.Object[Slot + 1];
		UserList[UserIndex].BancoInvent.Object[Slot + 1].ObjIndex = TempItem.ObjIndex;
		UserList[UserIndex].BancoInvent.Object[Slot + 1].Amount = TempItem.Amount;
	}

	UpdateBanUserInv(true, UserIndex, 0);
	UpdateVentanaBanco(UserIndex);

}

/* '' */
/* ' Handles the "ClanCodexUpdate" message. */
/* ' */
/* ' @param    userIndex The index of the user sending the message. */

void HandleClanCodexUpdate(int UserIndex) {
	/* '*************************************************** */
	/* 'Author: Juan Martín Sotuyo Dodero (Maraxus) */
	/* 'Last Modification: 05/17/06 */
	/* ' */
	/* '*************************************************** */
	if (UserList[UserIndex].sockctx->incomingData->length() < 5) {
		throw insufficient_data_error();
		return;
	}

	/* 'This packet contains strings, make a copy of the data to prevent losses if it's not complete yet... */
	clsByteQueue* buffer = UserList[UserIndex].sockctx->incomingData.get();


	/* 'Remove packet ID */
	buffer->ReadByte();

	std::string desc;
	std::vector<std::string> codex;

	desc = buffer->ReadUnicodeString();
	codex = vb6::Split(buffer->ReadUnicodeString(), SEPARATOR);

	ChangeCodexAndDesc(desc, codex, UserList[UserIndex].GuildIndex);

	/* 'If we got here then packet is complete, copy data back to original queue */

}

/* '' */
/* ' Handles the "UserCommerceOffer" message. */
/* ' */
/* ' @param    userIndex The index of the user sending the message. */

void HandleUserCommerceOffer(int UserIndex) {
	/* '*************************************************** */
	/* 'Author: Juan Martín Sotuyo Dodero (Maraxus) */
	/* 'Last Modification: 24/11/2009 */
	/* '24/11/2009: ZaMa - Nuevo sistema de comercio */
	/* '*************************************************** */
	if (UserList[UserIndex].sockctx->incomingData->length() < 7) {
		throw insufficient_data_error();
		return;
	}

	/* 'Remove packet ID */
	UserList[UserIndex].sockctx->incomingData->ReadByte();

	int Amount;
	int Slot;
	int tUser = 0;
	int OfferSlot;
	int ObjIndex = 0;

	Slot = UserList[UserIndex].sockctx->incomingData->ReadByte();
	Amount = UserList[UserIndex].sockctx->incomingData->ReadLong();
	OfferSlot = UserList[UserIndex].sockctx->incomingData->ReadByte();

	if (!PuedeSeguirComerciando(UserIndex)) {
		return;
	}

	/* 'Get the other player */
	tUser = UserList[UserIndex].ComUsu.DestUsu;

	/* ' If he's already confirmed his offer, but now tries to change it, then he's cheating */
	if (UserList[UserIndex].ComUsu.Confirmo == true) {

		/* ' Finish the trade */
		FinComerciarUsu(UserIndex);
		FinComerciarUsu(tUser);
		FlushBuffer(tUser);

		return;
	}

	/* 'If slot is invalid and it's not gold or it's not 0 (Substracting), then ignore it. */
	if (((Slot < 0 || Slot > UserList[UserIndex].CurrentInventorySlots) && Slot != FLAGORO)) {
		return;
	}

	/* 'If OfferSlot is invalid, then ignore it. */
	if (OfferSlot < 1 || OfferSlot > MAX_OFFER_SLOTS + 1) {
		return;
	}

	/* ' Can be negative if substracted from the offer, but never 0. */
	if (Amount == 0) {
		return;
	}

	/* 'Has he got enough?? */
	if (Slot == FLAGORO) {
		/* ' Can't offer more than he has */
		if (Amount > UserList[UserIndex].Stats.GLD - UserList[UserIndex].ComUsu.GoldAmount) {
			WriteCommerceChat(UserIndex, "No tienes esa cantidad de oro para agregar a la oferta.",
					FontTypeNames_FONTTYPE_TALK);
			return;
		}

		if (Amount < 0) {
			if (vb6::Abs(Amount) > UserList[UserIndex].ComUsu.GoldAmount) {
				Amount = UserList[UserIndex].ComUsu.GoldAmount * (-1);
			}
		}
	} else {
		/* 'If modifing a filled offerSlot, we already got the objIndex, then we don't need to know it */
		if (Slot != 0) {
			ObjIndex = UserList[UserIndex].Invent.Object[Slot].ObjIndex;
		}

		/* ' Non-Transferible or commerciable? */
		if (ObjIndex != 0) {
			if ((ObjData[ObjIndex].Intransferible == 1 || ObjData[ObjIndex].NoComerciable == 1)) {
				WriteCommerceChat(UserIndex, "No puedes comerciar este ítem.", FontTypeNames_FONTTYPE_TALK);
				return;
			}
		}

		/* ' Can't offer more than he has */
		if (!HasEnoughItems(UserIndex, ObjIndex, TotalOfferItems(ObjIndex, UserIndex) + Amount)) {

			WriteCommerceChat(UserIndex, "No tienes esa cantidad.", FontTypeNames_FONTTYPE_TALK);
			return;
		}

		if (Amount < 0) {
			if (vb6::Abs(Amount) > UserList[UserIndex].ComUsu.cant[OfferSlot]) {
				Amount = UserList[UserIndex].ComUsu.cant[OfferSlot] * (-1);
			}
		}

		if (ItemNewbie(ObjIndex)) {
			WriteCancelOfferItem(UserIndex, OfferSlot);
			return;
		}

		/* 'Don't allow to sell boats if they are equipped (you can't take them off in the water and causes trouble) */
		if (UserList[UserIndex].flags.Navegando == 1) {
			if (UserList[UserIndex].Invent.BarcoSlot == Slot) {
				WriteCommerceChat(UserIndex, "No puedes vender tu barco mientras lo estés usando.",
						FontTypeNames_FONTTYPE_TALK);
				return;
			}
		}

		if (UserList[UserIndex].Invent.MochilaEqpSlot > 0) {
			if (UserList[UserIndex].Invent.MochilaEqpSlot == Slot) {
				WriteCommerceChat(UserIndex, "No puedes vender tu mochila mientras la estés usando.",
						FontTypeNames_FONTTYPE_TALK);
				return;
			}
		}
	}

	AgregarOferta(UserIndex, OfferSlot, ObjIndex, Amount, Slot == FLAGORO);
	EnviarOferta(tUser, OfferSlot);

}

/* '' */
/* ' Handles the "GuildAcceptPeace" message. */
/* ' */
/* ' @param    userIndex The index of the user sending the message. */

void HandleGuildAcceptPeace(int UserIndex) {
	/* '*************************************************** */
	/* 'Author: Juan Martín Sotuyo Dodero (Maraxus) */
	/* 'Last Modification: 05/17/06 */
	/* ' */
	/* '*************************************************** */
	if (UserList[UserIndex].sockctx->incomingData->length() < 3) {
		throw insufficient_data_error();
		return;
	}

	/* 'This packet contains strings, make a copy of the data to prevent losses if it's not complete yet... */
	clsByteQueue* buffer = UserList[UserIndex].sockctx->incomingData.get();


	/* 'Remove packet ID */
	buffer->ReadByte();

	std::string guild;
	std::string errorStr;
	int otherClanIndex;

	guild = buffer->ReadUnicodeString();

	otherClanIndex = r_AceptarPropuestaDePaz(UserIndex, guild, errorStr);

	if (otherClanIndex == 0) {
		WriteConsoleMsg(UserIndex, errorStr, FontTypeNames_FONTTYPE_GUILD);
	} else {
		SendData(SendTarget_ToGuildMembers, UserList[UserIndex].GuildIndex,
				PrepareMessageConsoleMsg("Tu clan ha firmado la paz con " + guild + ".",
						FontTypeNames_FONTTYPE_GUILD));
		SendData(SendTarget_ToGuildMembers, otherClanIndex,
				PrepareMessageConsoleMsg(
						"Tu clan ha firmado la paz con " + GuildName(UserList[UserIndex].GuildIndex) + ".",
						FontTypeNames_FONTTYPE_GUILD));
	}

	/* 'If we got here then packet is complete, copy data back to original queue */

}

/* '' */
/* ' Handles the "GuildRejectAlliance" message. */
/* ' */
/* ' @param    userIndex The index of the user sending the message. */

void HandleGuildRejectAlliance(int UserIndex) {
	/* '*************************************************** */
	/* 'Author: Juan Martín Sotuyo Dodero (Maraxus) */
	/* 'Last Modification: 05/17/06 */
	/* ' */
	/* '*************************************************** */
	if (UserList[UserIndex].sockctx->incomingData->length() < 3) {
		throw insufficient_data_error();
		return;
	}

	/* 'This packet contains strings, make a copy of the data to prevent losses if it's not complete yet... */
	clsByteQueue* buffer = UserList[UserIndex].sockctx->incomingData.get();


	/* 'Remove packet ID */
	buffer->ReadByte();

	std::string guild;
	std::string errorStr;
	int otherClanIndex;

	guild = buffer->ReadUnicodeString();

	otherClanIndex = r_RechazarPropuestaDeAlianza(UserIndex, guild, errorStr);

	if (otherClanIndex == 0) {
		WriteConsoleMsg(UserIndex, errorStr, FontTypeNames_FONTTYPE_GUILD);
	} else {
		SendData(SendTarget_ToGuildMembers, UserList[UserIndex].GuildIndex,
				PrepareMessageConsoleMsg("Tu clan rechazado la propuesta de alianza de " + guild,
						FontTypeNames_FONTTYPE_GUILD));
		SendData(SendTarget_ToGuildMembers, otherClanIndex,
				PrepareMessageConsoleMsg(
						GuildName(UserList[UserIndex].GuildIndex)
								+ " ha rechazado nuestra propuesta de alianza con su clan.",
						FontTypeNames_FONTTYPE_GUILD));
	}

	/* 'If we got here then packet is complete, copy data back to original queue */


}

/* '' */
/* ' Handles the "GuildRejectPeace" message. */
/* ' */
/* ' @param    userIndex The index of the user sending the message. */

void HandleGuildRejectPeace(int UserIndex) {
	/* '*************************************************** */
	/* 'Author: Juan Martín Sotuyo Dodero (Maraxus) */
	/* 'Last Modification: 05/17/06 */
	/* ' */
	/* '*************************************************** */
	if (UserList[UserIndex].sockctx->incomingData->length() < 3) {
		throw insufficient_data_error();
		return;
	}

	/* 'This packet contains strings, make a copy of the data to prevent losses if it's not complete yet... */
	clsByteQueue* buffer = UserList[UserIndex].sockctx->incomingData.get();


	/* 'Remove packet ID */
	buffer->ReadByte();

	std::string guild;
	std::string errorStr;
	int otherClanIndex;

	guild = buffer->ReadUnicodeString();

	otherClanIndex = r_RechazarPropuestaDePaz(UserIndex, guild, errorStr);

	if (otherClanIndex == 0) {
		WriteConsoleMsg(UserIndex, errorStr, FontTypeNames_FONTTYPE_GUILD);
	} else {
		SendData(SendTarget_ToGuildMembers, UserList[UserIndex].GuildIndex,
				PrepareMessageConsoleMsg("Tu clan rechazado la propuesta de paz de " + guild + ".",
						FontTypeNames_FONTTYPE_GUILD));
		SendData(SendTarget_ToGuildMembers, otherClanIndex,
				PrepareMessageConsoleMsg(
						GuildName(UserList[UserIndex].GuildIndex)
								+ " ha rechazado nuestra propuesta de paz con su clan.",
						FontTypeNames_FONTTYPE_GUILD));
	}

	/* 'If we got here then packet is complete, copy data back to original queue */


}

/* '' */
/* ' Handles the "GuildAcceptAlliance" message. */
/* ' */
/* ' @param    userIndex The index of the user sending the message. */

void HandleGuildAcceptAlliance(int UserIndex) {
	/* '*************************************************** */
	/* 'Author: Juan Martín Sotuyo Dodero (Maraxus) */
	/* 'Last Modification: 05/17/06 */
	/* ' */
	/* '*************************************************** */
	if (UserList[UserIndex].sockctx->incomingData->length() < 3) {
		throw insufficient_data_error();
		return;
	}

	/* 'This packet contains strings, make a copy of the data to prevent losses if it's not complete yet... */
	clsByteQueue* buffer = UserList[UserIndex].sockctx->incomingData.get();


	/* 'Remove packet ID */
	buffer->ReadByte();

	std::string guild;
	std::string errorStr;
	int otherClanIndex;

	guild = buffer->ReadUnicodeString();

	otherClanIndex = r_AceptarPropuestaDeAlianza(UserIndex, guild, errorStr);

	if (otherClanIndex == 0) {
		WriteConsoleMsg(UserIndex, errorStr, FontTypeNames_FONTTYPE_GUILD);
	} else {
		SendData(SendTarget_ToGuildMembers, UserList[UserIndex].GuildIndex,
				PrepareMessageConsoleMsg("Tu clan ha firmado la alianza con " + guild + ".",
						FontTypeNames_FONTTYPE_GUILD));
		SendData(SendTarget_ToGuildMembers, otherClanIndex,
				PrepareMessageConsoleMsg(
						"Tu clan ha firmado la paz con " + GuildName(UserList[UserIndex].GuildIndex) + ".",
						FontTypeNames_FONTTYPE_GUILD));
	}

	/* 'If we got here then packet is complete, copy data back to original queue */


}

/* '' */
/* ' Handles the "GuildOfferPeace" message. */
/* ' */
/* ' @param    userIndex The index of the user sending the message. */

void HandleGuildOfferPeace(int UserIndex) {
	/* '*************************************************** */
	/* 'Author: Juan Martín Sotuyo Dodero (Maraxus) */
	/* 'Last Modification: 05/17/06 */
	/* ' */
	/* '*************************************************** */
	if (UserList[UserIndex].sockctx->incomingData->length() < 5) {
		throw insufficient_data_error();
		return;
	}

	/* 'This packet contains strings, make a copy of the data to prevent losses if it's not complete yet... */
	clsByteQueue* buffer = UserList[UserIndex].sockctx->incomingData.get();


	/* 'Remove packet ID */
	buffer->ReadByte();

	std::string guild;
	std::string proposal;
	std::string errorStr;

	guild = buffer->ReadUnicodeString();
	proposal = buffer->ReadUnicodeString();

	if (r_ClanGeneraPropuesta(UserIndex, guild, RELACIONES_GUILD_PAZ, proposal, errorStr)) {
		WriteConsoleMsg(UserIndex, "Propuesta de paz enviada.", FontTypeNames_FONTTYPE_GUILD);
	} else {
		WriteConsoleMsg(UserIndex, errorStr, FontTypeNames_FONTTYPE_GUILD);
	}

	/* 'If we got here then packet is complete, copy data back to original queue */


}

/* '' */
/* ' Handles the "GuildOfferAlliance" message. */
/* ' */
/* ' @param    userIndex The index of the user sending the message. */

void HandleGuildOfferAlliance(int UserIndex) {
	/* '*************************************************** */
	/* 'Author: Juan Martín Sotuyo Dodero (Maraxus) */
	/* 'Last Modification: 05/17/06 */
	/* ' */
	/* '*************************************************** */
	if (UserList[UserIndex].sockctx->incomingData->length() < 5) {
		throw insufficient_data_error();
		return;
	}

	/* 'This packet contains strings, make a copy of the data to prevent losses if it's not complete yet... */
	clsByteQueue* buffer = UserList[UserIndex].sockctx->incomingData.get();


	/* 'Remove packet ID */
	buffer->ReadByte();

	std::string guild;
	std::string proposal;
	std::string errorStr;

	guild = buffer->ReadUnicodeString();
	proposal = buffer->ReadUnicodeString();

	if (r_ClanGeneraPropuesta(UserIndex, guild, RELACIONES_GUILD_ALIADOS, proposal, errorStr)) {
		WriteConsoleMsg(UserIndex, "Propuesta de alianza enviada.", FontTypeNames_FONTTYPE_GUILD);
	} else {
		WriteConsoleMsg(UserIndex, errorStr, FontTypeNames_FONTTYPE_GUILD);
	}

	/* 'If we got here then packet is complete, copy data back to original queue */


}

/* '' */
/* ' Handles the "GuildAllianceDetails" message. */
/* ' */
/* ' @param    userIndex The index of the user sending the message. */

void HandleGuildAllianceDetails(int UserIndex) {
	/* '*************************************************** */
	/* 'Author: Juan Martín Sotuyo Dodero (Maraxus) */
	/* 'Last Modification: 05/17/06 */
	/* ' */
	/* '*************************************************** */
	if (UserList[UserIndex].sockctx->incomingData->length() < 3) {
		throw insufficient_data_error();
		return;
	}

	/* 'This packet contains strings, make a copy of the data to prevent losses if it's not complete yet... */
	clsByteQueue* buffer = UserList[UserIndex].sockctx->incomingData.get();


	/* 'Remove packet ID */
	buffer->ReadByte();

	std::string guild;
	std::string errorStr;
	std::string details;

	guild = buffer->ReadUnicodeString();

	details = r_VerPropuesta(UserIndex, guild, RELACIONES_GUILD_ALIADOS, errorStr);

	if (vb6::LenB(details) == 0) {
		WriteConsoleMsg(UserIndex, errorStr, FontTypeNames_FONTTYPE_GUILD);
	} else {
		WriteOfferDetails(UserIndex, details);
	}

	/* 'If we got here then packet is complete, copy data back to original queue */


}

/* '' */
/* ' Handles the "GuildPeaceDetails" message. */
/* ' */
/* ' @param    userIndex The index of the user sending the message. */

void HandleGuildPeaceDetails(int UserIndex) {
	/* '*************************************************** */
	/* 'Author: Juan Martín Sotuyo Dodero (Maraxus) */
	/* 'Last Modification: 05/17/06 */
	/* ' */
	/* '*************************************************** */
	if (UserList[UserIndex].sockctx->incomingData->length() < 3) {
		throw insufficient_data_error();
		return;
	}

	/* 'This packet contains strings, make a copy of the data to prevent losses if it's not complete yet... */
	clsByteQueue* buffer = UserList[UserIndex].sockctx->incomingData.get();


	/* 'Remove packet ID */
	buffer->ReadByte();

	std::string guild;
	std::string errorStr;
	std::string details;

	guild = buffer->ReadUnicodeString();

	details = r_VerPropuesta(UserIndex, guild, RELACIONES_GUILD_PAZ, errorStr);

	if (vb6::LenB(details) == 0) {
		WriteConsoleMsg(UserIndex, errorStr, FontTypeNames_FONTTYPE_GUILD);
	} else {
		WriteOfferDetails(UserIndex, details);
	}

	/* 'If we got here then packet is complete, copy data back to original queue */


}

/* '' */
/* ' Handles the "GuildRequestJoinerInfo" message. */
/* ' */
/* ' @param    userIndex The index of the user sending the message. */

void HandleGuildRequestJoinerInfo(int UserIndex) {
	/* '*************************************************** */
	/* 'Author: Juan Martín Sotuyo Dodero (Maraxus) */
	/* 'Last Modification: 05/17/06 */
	/* ' */
	/* '*************************************************** */
	if (UserList[UserIndex].sockctx->incomingData->length() < 3) {
		throw insufficient_data_error();
		return;
	}

	/* 'This packet contains strings, make a copy of the data to prevent losses if it's not complete yet... */
	clsByteQueue* buffer = UserList[UserIndex].sockctx->incomingData.get();


	/* 'Remove packet ID */
	buffer->ReadByte();

	std::string User;
	std::string details;

	User = buffer->ReadUnicodeString();

	details = a_DetallesAspirante(UserIndex, User);

	if (vb6::LenB(details) == 0) {
		WriteConsoleMsg(UserIndex, "El personaje no ha mandado solicitud, o no estás habilitado para verla.",
				FontTypeNames_FONTTYPE_GUILD);
	} else {
		WriteShowUserRequest(UserIndex, details);
	}

	/* 'If we got here then packet is complete, copy data back to original queue */


}

/* '' */
/* ' Handles the "GuildAlliancePropList" message. */
/* ' */
/* ' @param    userIndex The index of the user sending the message. */

void HandleGuildAlliancePropList(int UserIndex) {
	/* '*************************************************** */
	/* 'Author: Juan Martín Sotuyo Dodero (Maraxus) */
	/* 'Last Modification: 05/17/06 */
	/* ' */
	/* '*************************************************** */
	/* 'Remove packet ID */
	UserList[UserIndex].sockctx->incomingData->ReadByte();

	WriteAlianceProposalsList(UserIndex, r_ListaDePropuestas(UserIndex, RELACIONES_GUILD_ALIADOS));
}

/* '' */
/* ' Handles the "GuildPeacePropList" message. */
/* ' */
/* ' @param    userIndex The index of the user sending the message. */

void HandleGuildPeacePropList(int UserIndex) {
	/* '*************************************************** */
	/* 'Author: Juan Martín Sotuyo Dodero (Maraxus) */
	/* 'Last Modification: 05/17/06 */
	/* ' */
	/* '*************************************************** */
	/* 'Remove packet ID */
	UserList[UserIndex].sockctx->incomingData->ReadByte();

	WritePeaceProposalsList(UserIndex, r_ListaDePropuestas(UserIndex, RELACIONES_GUILD_PAZ));
}

/* '' */
/* ' Handles the "GuildDeclareWar" message. */
/* ' */
/* ' @param    userIndex The index of the user sending the message. */

void HandleGuildDeclareWar(int UserIndex) {
	/* '*************************************************** */
	/* 'Author: Juan Martín Sotuyo Dodero (Maraxus) */
	/* 'Last Modification: 05/17/06 */
	/* ' */
	/* '*************************************************** */
	if (UserList[UserIndex].sockctx->incomingData->length() < 3) {
		throw insufficient_data_error();
		return;
	}

	/* 'This packet contains strings, make a copy of the data to prevent losses if it's not complete yet... */
	clsByteQueue* buffer = UserList[UserIndex].sockctx->incomingData.get();


	/* 'Remove packet ID */
	buffer->ReadByte();

	std::string guild;
	std::string errorStr;
	int otherGuildIndex;

	guild = buffer->ReadUnicodeString();

	otherGuildIndex = r_DeclararGuerra(UserIndex, guild, errorStr);

	if (otherGuildIndex == 0) {
		WriteConsoleMsg(UserIndex, errorStr, FontTypeNames_FONTTYPE_GUILD);
	} else {
		/* 'WAR shall be! */
		SendData(SendTarget_ToGuildMembers, UserList[UserIndex].GuildIndex,
				PrepareMessageConsoleMsg("TU CLAN HA ENTRADO EN GUERRA CON " + guild + ".",
						FontTypeNames_FONTTYPE_GUILD));
		SendData(SendTarget_ToGuildMembers, otherGuildIndex,
				PrepareMessageConsoleMsg(
						GuildName(UserList[UserIndex].GuildIndex) + " LE DECLARA LA GUERRA A TU CLAN.",
						FontTypeNames_FONTTYPE_GUILD));
		SendData(SendTarget_ToGuildMembers, UserList[UserIndex].GuildIndex,
				PrepareMessagePlayWave(45, NO_3D_SOUND, NO_3D_SOUND));
		SendData(SendTarget_ToGuildMembers, otherGuildIndex,
				PrepareMessagePlayWave(45, NO_3D_SOUND, NO_3D_SOUND));
	}

	/* 'If we got here then packet is complete, copy data back to original queue */


}

/* '' */
/* ' Handles the "GuildNewWebsite" message. */
/* ' */
/* ' @param    userIndex The index of the user sending the message. */

void HandleGuildNewWebsite(int UserIndex) {
	/* '*************************************************** */
	/* 'Author: Juan Martín Sotuyo Dodero (Maraxus) */
	/* 'Last Modification: 05/17/06 */
	/* ' */
	/* '*************************************************** */
	if (UserList[UserIndex].sockctx->incomingData->length() < 3) {
		throw insufficient_data_error();
		return;
	}

	/* 'This packet contains strings, make a copy of the data to prevent losses if it's not complete yet... */
	clsByteQueue* buffer = UserList[UserIndex].sockctx->incomingData.get();


	/* 'Remove packet ID */
	buffer->ReadByte();

	ActualizarWebSite(UserIndex, buffer->ReadUnicodeString());

	/* 'If we got here then packet is complete, copy data back to original queue */


}

/* '' */
/* ' Handles the "GuildAcceptNewMember" message. */
/* ' */
/* ' @param    userIndex The index of the user sending the message. */

void HandleGuildAcceptNewMember(int UserIndex) {
	/* '*************************************************** */
	/* 'Author: Juan Martín Sotuyo Dodero (Maraxus) */
	/* 'Last Modification: 05/17/06 */
	/* ' */
	/* '*************************************************** */
	if (UserList[UserIndex].sockctx->incomingData->length() < 3) {
		throw insufficient_data_error();
		return;
	}

	/* 'This packet contains strings, make a copy of the data to prevent losses if it's not complete yet... */
	clsByteQueue* buffer = UserList[UserIndex].sockctx->incomingData.get();


	/* 'Remove packet ID */
	buffer->ReadByte();

	std::string errorStr;
	std::string UserName;
	int tUser = 0;

	UserName = buffer->ReadUnicodeString();

	if (!a_AceptarAspirante(UserIndex, UserName, errorStr)) {
		WriteConsoleMsg(UserIndex, errorStr, FontTypeNames_FONTTYPE_GUILD);
	} else {
		tUser = NameIndex(UserName);
		if (tUser > 0) {
			m_ConectarMiembroAClan(tUser, UserList[UserIndex].GuildIndex);
			RefreshCharStatus(tUser);
		}

		SendData(SendTarget_ToGuildMembers, UserList[UserIndex].GuildIndex,
				PrepareMessageConsoleMsg(UserName + " ha sido aceptado como miembro del clan.",
						FontTypeNames_FONTTYPE_GUILD));
		SendData(SendTarget_ToGuildMembers, UserList[UserIndex].GuildIndex,
				PrepareMessagePlayWave(43, NO_3D_SOUND, NO_3D_SOUND));
	}

	/* 'If we got here then packet is complete, copy data back to original queue */


}

/* '' */
/* ' Handles the "GuildRejectNewMember" message. */
/* ' */
/* ' @param    userIndex The index of the user sending the message. */

void HandleGuildRejectNewMember(int UserIndex) {
	/* '*************************************************** */
	/* 'Author: Juan Martín Sotuyo Dodero (Maraxus) */
	/* 'Last Modification: 01/08/07 */
	/* 'Last Modification by: (liquid) */
	/* ' */
	/* '*************************************************** */
	if (UserList[UserIndex].sockctx->incomingData->length() < 5) {
		throw insufficient_data_error();
		return;
	}

	/* 'This packet contains strings, make a copy of the data to prevent losses if it's not complete yet... */
	clsByteQueue* buffer = UserList[UserIndex].sockctx->incomingData.get();


	/* 'Remove packet ID */
	buffer->ReadByte();

	std::string errorStr;
	std::string UserName;
	std::string Reason;
	int tUser = 0;

	UserName = buffer->ReadUnicodeString();
	Reason = buffer->ReadUnicodeString();

	if (!a_RechazarAspirante(UserIndex, UserName, errorStr)) {
		WriteConsoleMsg(UserIndex, errorStr, FontTypeNames_FONTTYPE_GUILD);
	} else {
		tUser = NameIndex(UserName);

		if (tUser > 0) {
			WriteConsoleMsg(tUser, errorStr + " : " + Reason, FontTypeNames_FONTTYPE_GUILD);
		} else {
			/* 'hay que grabar en el char su rechazo */
			a_RechazarAspiranteChar(UserName, UserList[UserIndex].GuildIndex, Reason);
		}
	}

	/* 'If we got here then packet is complete, copy data back to original queue */


}

/* '' */
/* ' Handles the "GuildKickMember" message. */
/* ' */
/* ' @param    userIndex The index of the user sending the message. */

void HandleGuildKickMember(int UserIndex) {
	/* '*************************************************** */
	/* 'Author: Juan Martín Sotuyo Dodero (Maraxus) */
	/* 'Last Modification: 05/17/06 */
	/* ' */
	/* '*************************************************** */
	if (UserList[UserIndex].sockctx->incomingData->length() < 3) {
		throw insufficient_data_error();
		return;
	}

	/* 'This packet contains strings, make a copy of the data to prevent losses if it's not complete yet... */
	clsByteQueue* buffer = UserList[UserIndex].sockctx->incomingData.get();


	/* 'Remove packet ID */
	buffer->ReadByte();

	std::string UserName;
	int GuildIndex;

	UserName = buffer->ReadUnicodeString();

	GuildIndex = m_EcharMiembroDeClan(UserIndex, UserName);

	if (GuildIndex > 0) {
		SendData(SendTarget_ToGuildMembers, GuildIndex,
				PrepareMessageConsoleMsg(UserName + " fue expulsado del clan.",
						FontTypeNames_FONTTYPE_GUILD));
		SendData(SendTarget_ToGuildMembers, GuildIndex, PrepareMessagePlayWave(45, NO_3D_SOUND, NO_3D_SOUND));
	} else {
		WriteConsoleMsg(UserIndex, "No puedes expulsar ese personaje del clan.",
				FontTypeNames_FONTTYPE_GUILD);
	}

	/* 'If we got here then packet is complete, copy data back to original queue */


}

/* '' */
/* ' Handles the "GuildUpdateNews" message. */
/* ' */
/* ' @param    userIndex The index of the user sending the message. */

void HandleGuildUpdateNews(int UserIndex) {
	/* '*************************************************** */
	/* 'Author: Juan Martín Sotuyo Dodero (Maraxus) */
	/* 'Last Modification: 05/17/06 */
	/* ' */
	/* '*************************************************** */
	if (UserList[UserIndex].sockctx->incomingData->length() < 3) {
		throw insufficient_data_error();
		return;
	}

	/* 'This packet contains strings, make a copy of the data to prevent losses if it's not complete yet... */
	clsByteQueue* buffer = UserList[UserIndex].sockctx->incomingData.get();


	/* 'Remove packet ID */
	buffer->ReadByte();

	ActualizarNoticias(UserIndex, buffer->ReadUnicodeString());

	/* 'If we got here then packet is complete, copy data back to original queue */


}

/* '' */
/* ' Handles the "GuildMemberInfo" message. */
/* ' */
/* ' @param    userIndex The index of the user sending the message. */

void HandleGuildMemberInfo(int UserIndex) {
	/* '*************************************************** */
	/* 'Author: Juan Martín Sotuyo Dodero (Maraxus) */
	/* 'Last Modification: 05/17/06 */
	/* ' */
	/* '*************************************************** */
	if (UserList[UserIndex].sockctx->incomingData->length() < 3) {
		throw insufficient_data_error();
		return;
	}

	/* 'This packet contains strings, make a copy of the data to prevent losses if it's not complete yet... */
	clsByteQueue* buffer = UserList[UserIndex].sockctx->incomingData.get();


	/* 'Remove packet ID */
	buffer->ReadByte();

	SendDetallesPersonaje(UserIndex, buffer->ReadUnicodeString());

	/* 'If we got here then packet is complete, copy data back to original queue */


}

/* '' */
/* ' Handles the "GuildOpenElections" message. */
/* ' */
/* ' @param    userIndex The index of the user sending the message. */

void HandleGuildOpenElections(int UserIndex) {
	/* '*************************************************** */
	/* 'Author: Juan Martín Sotuyo Dodero (Maraxus) */
	/* 'Last Modification: 05/17/06 */
	/* ' */
	/* '*************************************************** */
	/* 'Remove packet ID */
	UserList[UserIndex].sockctx->incomingData->ReadByte();

	std::string ERROR;

	if (!v_AbrirElecciones(UserIndex, ERROR)) {
		WriteConsoleMsg(UserIndex, ERROR, FontTypeNames_FONTTYPE_GUILD);
	} else {
		SendData(SendTarget_ToGuildMembers, UserList[UserIndex].GuildIndex,
				PrepareMessageConsoleMsg(
						"¡Han comenzado las elecciones del clan! Puedes votar escribiendo /VOTO seguido del nombre del personaje, por ejemplo: /VOTO "
								+ UserList[UserIndex].Name, FontTypeNames_FONTTYPE_GUILD));
	}
}

/* '' */
/* ' Handles the "GuildRequestMembership" message. */
/* ' */
/* ' @param    userIndex The index of the user sending the message. */

void HandleGuildRequestMembership(int UserIndex) {
	/* '*************************************************** */
	/* 'Author: Juan Martín Sotuyo Dodero (Maraxus) */
	/* 'Last Modification: 05/17/06 */
	/* ' */
	/* '*************************************************** */
	if (UserList[UserIndex].sockctx->incomingData->length() < 5) {
		throw insufficient_data_error();
		return;
	}

	/* 'This packet contains strings, make a copy of the data to prevent losses if it's not complete yet... */
	clsByteQueue* buffer = UserList[UserIndex].sockctx->incomingData.get();


	/* 'Remove packet ID */
	buffer->ReadByte();

	std::string guild;
	std::string application;
	std::string errorStr;

	guild = buffer->ReadUnicodeString();
	application = buffer->ReadUnicodeString();

	if (!a_NuevoAspirante(UserIndex, guild, application, errorStr)) {
		WriteConsoleMsg(UserIndex, errorStr, FontTypeNames_FONTTYPE_GUILD);
	} else {
		WriteConsoleMsg(UserIndex,
				"Tu solicitud ha sido enviada. Espera prontas noticias del líder de " + guild + ".",
				FontTypeNames_FONTTYPE_GUILD);
	}

	/* 'If we got here then packet is complete, copy data back to original queue */


}

/* '' */
/* ' Handles the "GuildRequestDetails" message. */
/* ' */
/* ' @param    userIndex The index of the user sending the message. */

void HandleGuildRequestDetails(int UserIndex) {
	/* '*************************************************** */
	/* 'Author: Juan Martín Sotuyo Dodero (Maraxus) */
	/* 'Last Modification: 05/17/06 */
	/* ' */
	/* '*************************************************** */
	if (UserList[UserIndex].sockctx->incomingData->length() < 3) {
		throw insufficient_data_error();
		return;
	}

	/* 'This packet contains strings, make a copy of the data to prevent losses if it's not complete yet... */
	clsByteQueue* buffer = UserList[UserIndex].sockctx->incomingData.get();


	/* 'Remove packet ID */
	buffer->ReadByte();

	SendGuildDetails(UserIndex, buffer->ReadUnicodeString());

	/* 'If we got here then packet is complete, copy data back to original queue */


}

/* '' */
/* ' Handles the "Online" message. */
/* ' */
/* ' @param    userIndex The index of the user sending the message. */

void HandleOnline(int UserIndex) {
	/* '*************************************************** */
	/* 'Author: Juan Martín Sotuyo Dodero (Maraxus) */
	/* 'Last Modification: 05/17/06 */
	/* ' */
	/* '*************************************************** */
	int i;
	int Count = 0;

	/* 'Remove packet ID */
	UserList[UserIndex].sockctx->incomingData->ReadByte();

	for (i = (1); i <= (LastUser); i++) {
		if (vb6::LenB(UserList[i].Name) != 0) {
			if (UserTieneAlgunPrivilegios(i, PlayerType_User, PlayerType_Consejero)) {
				Count = Count + 1;
			}
		}
	}

	WriteConsoleMsg(UserIndex, "Número de usuarios: " + vb6::CStr(Count), FontTypeNames_FONTTYPE_INFO);
}

/* '' */
/* ' Handles the "Quit" message. */
/* ' */
/* ' @param    userIndex The index of the user sending the message. */

void HandleQuit(int UserIndex) {
	/* '*************************************************** */
	/* 'Author: Juan Martín Sotuyo Dodero (Maraxus) */
	/* 'Last Modification: 04/15/2008 (NicoNZ) */
	/* 'If user is invisible, it automatically becomes */
	/* 'visible before doing the countdown to exit */
	/* '04/15/2008 - No se reseteaban lso contadores de invi ni de ocultar. (NicoNZ) */
	/* '*************************************************** */

	/* 'Remove packet ID */
	UserList[UserIndex].sockctx->incomingData->ReadByte();

	if (UserList[UserIndex].flags.Paralizado == 1) {
		WriteConsoleMsg(UserIndex, "No puedes salir estando paralizado.", FontTypeNames_FONTTYPE_WARNING);
		return;
	}

	CerrarUserIndexIniciar(UserIndex);
}

/* '' */
/* ' Handles the "GuildLeave" message. */
/* ' */
/* ' @param    userIndex The index of the user sending the message. */

void HandleGuildLeave(int UserIndex) {
	/* '*************************************************** */
	/* 'Author: Juan Martín Sotuyo Dodero (Maraxus) */
	/* 'Last Modification: 05/17/06 */
	/* ' */
	/* '*************************************************** */
	int GuildIndex;

	/* 'Remove packet ID */
	UserList[UserIndex].sockctx->incomingData->ReadByte();

	/* 'obtengo el guildindex */
	GuildIndex = m_EcharMiembroDeClan(UserIndex, UserList[UserIndex].Name);

	if (GuildIndex > 0) {
		WriteConsoleMsg(UserIndex, "Dejas el clan.", FontTypeNames_FONTTYPE_GUILD);
		SendData(SendTarget_ToGuildMembers, GuildIndex,
				PrepareMessageConsoleMsg(UserList[UserIndex].Name + " deja el clan.",
						FontTypeNames_FONTTYPE_GUILD));
	} else {
		WriteConsoleMsg(UserIndex, "Tú no puedes salir de este clan.", FontTypeNames_FONTTYPE_GUILD);
	}
}

/* '' */
/* ' Handles the "RequestAccountState" message. */
/* ' */
/* ' @param    userIndex The index of the user sending the message. */

void HandleRequestAccountState(int UserIndex) {
	/* '*************************************************** */
	/* 'Author: Juan Martín Sotuyo Dodero (Maraxus) */
	/* 'Last Modification: 05/17/06 */
	/* ' */
	/* '*************************************************** */
	int earnings = 0;
	int Percentage = 0;

	/* 'Remove packet ID */
	UserList[UserIndex].sockctx->incomingData->ReadByte();

	/* 'Dead people can't check their accounts */
	if (UserList[UserIndex].flags.Muerto == 1) {
		WriteConsoleMsg(UserIndex, "¡¡Estás muerto!!", FontTypeNames_FONTTYPE_INFO);
		return;
	}

	/* 'Validate target NPC */
	if (UserList[UserIndex].flags.TargetNPC == 0) {
		WriteConsoleMsg(UserIndex,
				"Primero tienes que seleccionar un personaje, haz click izquierdo sobre él.",
				FontTypeNames_FONTTYPE_INFO);
		return;
	}

	if (Distancia(Npclist[UserList[UserIndex].flags.TargetNPC].Pos, UserList[UserIndex].Pos) > 3) {
		WriteConsoleMsg(UserIndex, "Estás demasiado lejos del vendedor.", FontTypeNames_FONTTYPE_INFO);
		return;
	}

	switch (Npclist[UserList[UserIndex].flags.TargetNPC].NPCtype) {
	case eNPCType_Banquero:
		WriteChatOverHead(UserIndex,
				"Tienes " + vb6::CStr(UserList[UserIndex].Stats.Banco) + " monedas de oro en tu cuenta.",
				Npclist[UserList[UserIndex].flags.TargetNPC].Char.CharIndex, 0x00ffffff);

		break;

	case eNPCType_Timbero:
		if (!UserTieneAlgunPrivilegios(UserIndex, PlayerType_User)) {
			earnings = Apuestas.Ganancias - Apuestas.Perdidas;

			if (earnings >= 0 && Apuestas.Ganancias != 0) {
				Percentage = vb6::Int(earnings * 100 / Apuestas.Ganancias);
			}

			if (earnings < 0 && Apuestas.Perdidas != 0) {
				Percentage = vb6::Int(earnings * 100 / Apuestas.Perdidas);
			}

			WriteConsoleMsg(UserIndex,
					"Entradas: " + vb6::CStr(Apuestas.Ganancias) + " Salida: " + vb6::CStr(Apuestas.Perdidas) + " Ganancia Neta: "
							+ vb6::CStr(earnings) + " (" + vb6::CStr(Percentage) + "%) Jugadas: " + vb6::CStr(Apuestas.Jugadas),
					FontTypeNames_FONTTYPE_INFO);
		}
		break;

	default:
		break;
	}
}

/* '' */
/* ' Handles the "PetStand" message. */
/* ' */
/* ' @param    userIndex The index of the user sending the message. */

void HandlePetStand(int UserIndex) {
	/* '*************************************************** */
	/* 'Author: Juan Martín Sotuyo Dodero (Maraxus) */
	/* 'Last Modification: 05/17/06 */
	/* ' */
	/* '*************************************************** */
	/* 'Remove packet ID */
	UserList[UserIndex].sockctx->incomingData->ReadByte();

	/* 'Dead people can't use pets */
	if (UserList[UserIndex].flags.Muerto == 1) {
		WriteConsoleMsg(UserIndex, "¡¡Estás muerto!!", FontTypeNames_FONTTYPE_INFO);
		return;
	}

	/* 'Validate target NPC */
	if (UserList[UserIndex].flags.TargetNPC == 0) {
		WriteConsoleMsg(UserIndex,
				"Primero tienes que seleccionar un personaje, haz click izquierdo sobre él.",
				FontTypeNames_FONTTYPE_INFO);
		return;
	}

	/* 'Make sure it's close enough */
	if (Distancia(Npclist[UserList[UserIndex].flags.TargetNPC].Pos, UserList[UserIndex].Pos) > 10) {
		WriteConsoleMsg(UserIndex, "Estás demasiado lejos.", FontTypeNames_FONTTYPE_INFO);
		return;
	}

	/* 'Make sure it's his pet */
	if (Npclist[UserList[UserIndex].flags.TargetNPC].MaestroUser != UserIndex) {
		return;
	}

	/* 'Do it! */
	Npclist[UserList[UserIndex].flags.TargetNPC].Movement = TipoAI_ESTATICO;

	Expresar(UserList[UserIndex].flags.TargetNPC, UserIndex);
}

/* '' */
/* ' Handles the "PetFollow" message. */
/* ' */
/* ' @param    userIndex The index of the user sending the message. */

void HandlePetFollow(int UserIndex) {
	/* '*************************************************** */
	/* 'Author: Juan Martín Sotuyo Dodero (Maraxus) */
	/* 'Last Modification: 05/17/06 */
	/* ' */
	/* '*************************************************** */
	/* 'Remove packet ID */
	UserList[UserIndex].sockctx->incomingData->ReadByte();

	/* 'Dead users can't use pets */
	if (UserList[UserIndex].flags.Muerto == 1) {
		WriteConsoleMsg(UserIndex, "¡¡Estás muerto!!", FontTypeNames_FONTTYPE_INFO);
		return;
	}

	/* 'Validate target NPC */
	if (UserList[UserIndex].flags.TargetNPC == 0) {
		WriteConsoleMsg(UserIndex,
				"Primero tienes que seleccionar un personaje, haz click izquierdo sobre él.",
				FontTypeNames_FONTTYPE_INFO);
		return;
	}

	/* 'Make sure it's close enough */
	if (Distancia(Npclist[UserList[UserIndex].flags.TargetNPC].Pos, UserList[UserIndex].Pos) > 10) {
		WriteConsoleMsg(UserIndex, "Estás demasiado lejos.", FontTypeNames_FONTTYPE_INFO);
		return;
	}

	/* 'Make usre it's the user's pet */
	if (Npclist[UserList[UserIndex].flags.TargetNPC].MaestroUser != UserIndex) {
		return;
	}

	/* 'Do it */
	FollowAmo(UserList[UserIndex].flags.TargetNPC);

	Expresar(UserList[UserIndex].flags.TargetNPC, UserIndex);
}

/* '' */
/* ' Handles the "ReleasePet" message. */
/* ' */
/* ' @param    userIndex The index of the user sending the message. */

void HandleReleasePet(int UserIndex) {
	/* '*************************************************** */
	/* 'Author: ZaMa */
	/* 'Last Modification: 18/11/2009 */
	/* ' */
	/* '*************************************************** */
	/* 'Remove packet ID */
	UserList[UserIndex].sockctx->incomingData->ReadByte();

	/* 'Dead users can't use pets */
	if (UserList[UserIndex].flags.Muerto == 1) {
		WriteConsoleMsg(UserIndex, "¡¡Estás muerto!!", FontTypeNames_FONTTYPE_INFO);
		return;
	}

	/* 'Validate target NPC */
	if (UserList[UserIndex].flags.TargetNPC == 0) {
		WriteConsoleMsg(UserIndex,
				"Primero tienes que seleccionar una mascota, haz click izquierdo sobre ella.",
				FontTypeNames_FONTTYPE_INFO);
		return;
	}

	/* 'Make usre it's the user's pet */
	if (Npclist[UserList[UserIndex].flags.TargetNPC].MaestroUser != UserIndex) {
		return;
	}

	/* 'Make sure it's close enough */
	if (Distancia(Npclist[UserList[UserIndex].flags.TargetNPC].Pos, UserList[UserIndex].Pos) > 10) {
		WriteConsoleMsg(UserIndex, "Estás demasiado lejos.", FontTypeNames_FONTTYPE_INFO);
		return;
	}

	/* 'Do it */
	QuitarPet(UserIndex, UserList[UserIndex].flags.TargetNPC);

}

/* '' */
/* ' Handles the "TrainList" message. */
/* ' */
/* ' @param    userIndex The index of the user sending the message. */

void HandleTrainList(int UserIndex) {
	/* '*************************************************** */
	/* 'Author: Juan Martín Sotuyo Dodero (Maraxus) */
	/* 'Last Modification: 05/17/06 */
	/* ' */
	/* '*************************************************** */
	/* 'Remove packet ID */
	UserList[UserIndex].sockctx->incomingData->ReadByte();

	/* 'Dead users can't use pets */
	if (UserList[UserIndex].flags.Muerto == 1) {
		WriteConsoleMsg(UserIndex, "¡¡Estás muerto!!", FontTypeNames_FONTTYPE_INFO);
		return;
	}

	/* 'Validate target NPC */
	if (UserList[UserIndex].flags.TargetNPC == 0) {
		WriteConsoleMsg(UserIndex,
				"Primero tienes que seleccionar un personaje, haz click izquierdo sobre él.",
				FontTypeNames_FONTTYPE_INFO);
		return;
	}

	/* 'Make sure it's close enough */
	if (Distancia(Npclist[UserList[UserIndex].flags.TargetNPC].Pos, UserList[UserIndex].Pos) > 10) {
		WriteConsoleMsg(UserIndex, "Estás demasiado lejos.", FontTypeNames_FONTTYPE_INFO);
		return;
	}

	/* 'Make sure it's the trainer */
	if (Npclist[UserList[UserIndex].flags.TargetNPC].NPCtype != eNPCType_Entrenador) {
		return;
	}

	WriteTrainerCreatureList(UserIndex, UserList[UserIndex].flags.TargetNPC);
}

/* '' */
/* ' Handles the "Rest" message. */
/* ' */
/* ' @param    userIndex The index of the user sending the message. */

void HandleRest(int UserIndex) {
	/* '*************************************************** */
	/* 'Author: Juan Martín Sotuyo Dodero (Maraxus) */
	/* 'Last Modification: 05/17/06 */
	/* ' */
	/* '*************************************************** */
	/* 'Remove packet ID */
	UserList[UserIndex].sockctx->incomingData->ReadByte();

	/* 'Dead users can't use pets */
	if (UserList[UserIndex].flags.Muerto == 1) {
		WriteConsoleMsg(UserIndex, "¡¡Estás muerto!! Solo puedes usar ítems cuando estás vivo.",
				FontTypeNames_FONTTYPE_INFO);
		return;
	}

	if (HayOBJarea(UserList[UserIndex].Pos, FOGATA)) {
		WriteRestOK(UserIndex);

		if (!UserList[UserIndex].flags.Descansar) {
			WriteConsoleMsg(UserIndex, "Te acomodás junto a la fogata y comienzas a descansar.",
					FontTypeNames_FONTTYPE_INFO);
		} else {
			WriteConsoleMsg(UserIndex, "Te levantas.", FontTypeNames_FONTTYPE_INFO);
		}

		UserList[UserIndex].flags.Descansar = !UserList[UserIndex].flags.Descansar;
	} else {
		if (UserList[UserIndex].flags.Descansar) {
			WriteRestOK(UserIndex);
			WriteConsoleMsg(UserIndex, "Te levantas.", FontTypeNames_FONTTYPE_INFO);

			UserList[UserIndex].flags.Descansar = false;
			return;
		}

		WriteConsoleMsg(UserIndex, "No hay ninguna fogata junto a la cual descansar.",
				FontTypeNames_FONTTYPE_INFO);
	}
}

/* '' */
/* ' Handles the "Meditate" message. */
/* ' */
/* ' @param    userIndex The index of the user sending the message. */

void HandleMeditate(int UserIndex) {
	/* '*************************************************** */
	/* 'Author: Juan Martín Sotuyo Dodero (Maraxus) */
	/* 'Last Modification: 04/15/08 (NicoNZ) */
	/* 'Arreglé un bug que mandaba un index de la meditacion diferente */
	/* 'al que decia el server. */
	/* '*************************************************** */
	/* 'Remove packet ID */
	UserList[UserIndex].sockctx->incomingData->ReadByte();

	/* 'Dead users can't use pets */
	if (UserList[UserIndex].flags.Muerto == 1) {
		WriteConsoleMsg(UserIndex, "¡¡Estás muerto!! Sólo puedes meditar cuando estás vivo.",
				FontTypeNames_FONTTYPE_INFO);
		return;
	}

	/* 'Can he meditate? */
	if (UserList[UserIndex].Stats.MaxMAN == 0) {
		WriteConsoleMsg(UserIndex, "Sólo las clases mágicas conocen el arte de la meditación.",
				FontTypeNames_FONTTYPE_INFO);
		return;
	}

	/* 'Admins don't have to wait :D */
	if (!UserTieneAlgunPrivilegios(UserIndex, PlayerType_User)) {
		UserList[UserIndex].Stats.MinMAN = UserList[UserIndex].Stats.MaxMAN;
		WriteConsoleMsg(UserIndex, "Maná restaurado.", FontTypeNames_FONTTYPE_VENENO);
		WriteUpdateMana(UserIndex);
		return;
	}

	WriteMeditateToggle(UserIndex);

	if (UserList[UserIndex].flags.Meditando) {
		WriteConsoleMsg(UserIndex, "Dejas de meditar.", FontTypeNames_FONTTYPE_INFO);
	}

	UserList[UserIndex].flags.Meditando = !UserList[UserIndex].flags.Meditando;

	/* 'Barrin 3/10/03 Tiempo de inicio al meditar */
	if (UserList[UserIndex].flags.Meditando) {
		UserList[UserIndex].Counters.tInicioMeditar = vb6::GetTickCount();

		/* 'Call WriteConsoleMsg(UserIndex, "Te estás concentrando. En " & Fix(TIEMPO_INICIOMEDITAR / 1000) & " segundos comenzarás a meditar.", FontTypeNames.FONTTYPE_INFO) */
		/* ' [TEMPORAL] */
		WriteConsoleMsg(UserIndex,
				"Te estás concentrando. En " + vb6::CStr(UserList[UserIndex].Stats.ELV / 17)
						+ " segundos comenzarás a meditar.", FontTypeNames_FONTTYPE_INFO);

		UserList[UserIndex].Char.loops = INFINITE_LOOPS;

		/* 'Show proper FX according to level */
		if (UserList[UserIndex].Stats.ELV < 13) {
			UserList[UserIndex].Char.FX = FXIDs_FXMEDITARCHICO;

		} else if (UserList[UserIndex].Stats.ELV < 25) {
			UserList[UserIndex].Char.FX = FXIDs_FXMEDITARMEDIANO;

		} else if (UserList[UserIndex].Stats.ELV < 35) {
			UserList[UserIndex].Char.FX = FXIDs_FXMEDITARGRANDE;

		} else if (UserList[UserIndex].Stats.ELV < 42) {
			UserList[UserIndex].Char.FX = FXIDs_FXMEDITARXGRANDE;

		} else {
			UserList[UserIndex].Char.FX = FXIDs_FXMEDITARXXGRANDE;
		}

		SendData(SendTarget_ToPCArea, UserIndex,
				PrepareMessageCreateFX(UserList[UserIndex].Char.CharIndex, UserList[UserIndex].Char.FX,
						INFINITE_LOOPS));
	} else {
		UserList[UserIndex].Counters.bPuedeMeditar = false;

		UserList[UserIndex].Char.FX = 0;
		UserList[UserIndex].Char.loops = 0;
		SendData(SendTarget_ToPCArea, UserIndex,
				PrepareMessageCreateFX(UserList[UserIndex].Char.CharIndex, 0, 0));
	}
}

/* '' */
/* ' Handles the "Resucitate" message. */
/* ' */
/* ' @param    userIndex The index of the user sending the message. */

void HandleResucitate(int UserIndex) {
	/* '*************************************************** */
	/* 'Author: Juan Martín Sotuyo Dodero (Maraxus) */
	/* 'Last Modification: 05/17/06 */
	/* ' */
	/* '*************************************************** */
	/* 'Remove packet ID */
	UserList[UserIndex].sockctx->incomingData->ReadByte();

	/* 'Se asegura que el target es un npc */
	if (UserList[UserIndex].flags.TargetNPC == 0) {
		WriteConsoleMsg(UserIndex,
				"Primero tienes que seleccionar un personaje, haz click izquierdo sobre él.",
				FontTypeNames_FONTTYPE_INFO);
		return;
	}

	/* 'Validate NPC and make sure player is dead */
	if ((Npclist[UserList[UserIndex].flags.TargetNPC].NPCtype != eNPCType_Revividor
			&& (Npclist[UserList[UserIndex].flags.TargetNPC].NPCtype != eNPCType_ResucitadorNewbie
					|| !EsNewbie(UserIndex))) || UserList[UserIndex].flags.Muerto == 0) {
		return;
	}

	/* 'Make sure it's close enough */
	if (Distancia(UserList[UserIndex].Pos, Npclist[UserList[UserIndex].flags.TargetNPC].Pos) > 10) {
		WriteConsoleMsg(UserIndex, "El sacerdote no puede resucitarte debido a que estás demasiado lejos.",
				FontTypeNames_FONTTYPE_INFO);
		return;
	}

	RevivirUsuario(UserIndex);
	WriteConsoleMsg(UserIndex, "¡¡Has sido resucitado!!", FontTypeNames_FONTTYPE_INFO);

}

/* '' */
/* ' Handles the "Consultation" message. */
/* ' */
/* ' @param    userIndex The index of the user sending the message. */

void HandleConsultation(int UserIndex) {
	/* '*************************************************** */
	/* 'Author: ZaMa */
	/* 'Last Modification: 01/05/2010 */
	/* 'Habilita/Deshabilita el modo consulta. */
	/* '01/05/2010: ZaMa - Agrego validaciones. */
	/* '16/09/2010: ZaMa - No se hace visible en los clientes si estaba navegando (porque ya lo estaba). */
	/* '*************************************************** */

	int UserConsulta;

	/* 'Remove packet ID */
	UserList[UserIndex].sockctx->incomingData->ReadByte();

	/* ' Comando exclusivo para gms */
	if (!EsGm(UserIndex)) {
		return;
	}

	UserConsulta = UserList[UserIndex].flags.TargetUser;

	/* 'Se asegura que el target es un usuario */
	if (UserConsulta == 0) {
		WriteConsoleMsg(UserIndex, "Primero tienes que seleccionar un usuario, haz click izquierdo sobre él.",
				FontTypeNames_FONTTYPE_INFO);
		return;
	}

	/* ' No podes ponerte a vos mismo en modo consulta. */
	if (UserConsulta == UserIndex) {
		return;
	}

	/* ' No podes estra en consulta con otro gm */
	if (EsGm(UserConsulta)) {
		WriteConsoleMsg(UserIndex, "No puedes iniciar el modo consulta con otro administrador.",
				FontTypeNames_FONTTYPE_INFO);
		return;
	}

	std::string UserName;
	UserName = UserList[UserConsulta].Name;

	/* ' Si ya estaba en consulta, termina la consulta */
	if (UserList[UserConsulta].flags.EnConsulta) {
		WriteConsoleMsg(UserIndex, "Has terminado el modo consulta con " + UserName + ".",
				FontTypeNames_FONTTYPE_INFOBOLD);
		WriteConsoleMsg(UserConsulta, "Has terminado el modo consulta.", FontTypeNames_FONTTYPE_INFOBOLD);
		LogGM(UserList[UserIndex].Name, "Termino consulta con " + UserName);

		UserList[UserConsulta].flags.EnConsulta = false;

		/* ' Sino la inicia */
	} else {
		WriteConsoleMsg(UserIndex, "Has iniciado el modo consulta con " + UserName + ".",
				FontTypeNames_FONTTYPE_INFOBOLD);
		WriteConsoleMsg(UserConsulta, "Has iniciado el modo consulta.", FontTypeNames_FONTTYPE_INFOBOLD);
		LogGM(UserList[UserIndex].Name, "Inicio consulta con " + UserName);

		UserList[UserConsulta].flags.EnConsulta = true;

		/* ' Pierde invi u ocu */
		if (UserList[UserConsulta].flags.invisible == 1 || UserList[UserConsulta].flags.Oculto == 1) {
			UserList[UserConsulta].flags.Oculto = 0;
			UserList[UserConsulta].flags.invisible = 0;
			UserList[UserConsulta].Counters.TiempoOculto = 0;
			UserList[UserConsulta].Counters.Invisibilidad = 0;

			if (UserList[UserConsulta].flags.Navegando == 0) {
				SetInvisible(UserConsulta, UserList[UserConsulta].Char.CharIndex, false);
			}
		}
	}

	SetConsulatMode(UserConsulta);

}

/* '' */
/* ' Handles the "Heal" message. */
/* ' */
/* ' @param    userIndex The index of the user sending the message. */

void HandleHeal(int UserIndex) {
	/* '*************************************************** */
	/* 'Author: Juan Martín Sotuyo Dodero (Maraxus) */
	/* 'Last Modification: 05/17/06 */
	/* ' */
	/* '*************************************************** */
	/* 'Remove packet ID */
	UserList[UserIndex].sockctx->incomingData->ReadByte();

	/* 'Se asegura que el target es un npc */
	if (UserList[UserIndex].flags.TargetNPC == 0) {
		WriteConsoleMsg(UserIndex,
				"Primero tienes que seleccionar un personaje, haz click izquierdo sobre él.",
				FontTypeNames_FONTTYPE_INFO);
		return;
	}

	if ((Npclist[UserList[UserIndex].flags.TargetNPC].NPCtype != eNPCType_Revividor
			&& Npclist[UserList[UserIndex].flags.TargetNPC].NPCtype != eNPCType_ResucitadorNewbie)
			|| UserList[UserIndex].flags.Muerto != 0) {
		return;
	}

	if (Distancia(UserList[UserIndex].Pos, Npclist[UserList[UserIndex].flags.TargetNPC].Pos) > 10) {
		WriteConsoleMsg(UserIndex, "El sacerdote no puede curarte debido a que estás demasiado lejos.",
				FontTypeNames_FONTTYPE_INFO);
		return;
	}

	UserList[UserIndex].Stats.MinHp = UserList[UserIndex].Stats.MaxHp;

	WriteUpdateHP(UserIndex);

	WriteConsoleMsg(UserIndex, "¡¡Has sido curado!!", FontTypeNames_FONTTYPE_INFO);
}

/* '' */
/* ' Handles the "RequestStats" message. */
/* ' */
/* ' @param    userIndex The index of the user sending the message. */

void HandleRequestStats(int UserIndex) {
	/* '*************************************************** */
	/* 'Author: Juan Martín Sotuyo Dodero (Maraxus) */
	/* 'Last Modification: 05/17/06 */
	/* ' */
	/* '*************************************************** */
	/* 'Remove packet ID */
	UserList[UserIndex].sockctx->incomingData->ReadByte();

	SendUserStatsTxt(UserIndex, UserIndex);
}

/* '' */
/* ' Handles the "Help" message. */
/* ' */
/* ' @param    userIndex The index of the user sending the message. */

void HandleHelp(int UserIndex) {
	/* '*************************************************** */
	/* 'Author: Juan Martín Sotuyo Dodero (Maraxus) */
	/* 'Last Modification: 05/17/06 */
	/* ' */
	/* '*************************************************** */
	/* 'Remove packet ID */
	UserList[UserIndex].sockctx->incomingData->ReadByte();

	SendHelp(UserIndex);
}

/* '' */
/* ' Handles the "CommerceStart" message. */
/* ' */
/* ' @param    userIndex The index of the user sending the message. */

void HandleCommerceStart(int UserIndex) {
	/* '*************************************************** */
	/* 'Author: Juan Martín Sotuyo Dodero (Maraxus) */
	/* 'Last Modification: 05/17/06 */
	/* ' */
	/* '*************************************************** */
	int i;
	/* 'Remove packet ID */
	UserList[UserIndex].sockctx->incomingData->ReadByte();

	/* 'Dead people can't commerce */
	if (UserList[UserIndex].flags.Muerto == 1) {
		WriteConsoleMsg(UserIndex, "¡¡Estás muerto!!", FontTypeNames_FONTTYPE_INFO);
		return;
	}

	/* 'Is it already in commerce mode?? */
	if (UserList[UserIndex].flags.Comerciando) {
		WriteConsoleMsg(UserIndex, "Ya estás comerciando.", FontTypeNames_FONTTYPE_INFO);
		return;
	}

	/* 'Validate target NPC */
	if (UserList[UserIndex].flags.TargetNPC > 0) {
		/* 'Does the NPC want to trade?? */
		if (Npclist[UserList[UserIndex].flags.TargetNPC].Comercia == 0) {
			if (vb6::LenB(Npclist[UserList[UserIndex].flags.TargetNPC].desc) != 0) {
				WriteChatOverHead(UserIndex, "No tengo ningún interés en comerciar.",
						Npclist[UserList[UserIndex].flags.TargetNPC].Char.CharIndex, 0x00ffffff);
			}

			return;
		}

		if (Distancia(Npclist[UserList[UserIndex].flags.TargetNPC].Pos, UserList[UserIndex].Pos) > 3) {
			WriteConsoleMsg(UserIndex, "Estás demasiado lejos del vendedor.", FontTypeNames_FONTTYPE_INFO);
			return;
		}

		/* 'Start commerce.... */
		IniciarComercioNPC(UserIndex);
		/* '[Alejo] */
	} else if (UserList[UserIndex].flags.TargetUser > 0) {
		/* 'User commerce... */
		/* 'Can he commerce?? */
		if (UserTieneAlgunPrivilegios(UserIndex, PlayerType_Consejero)) {
			WriteConsoleMsg(UserIndex, "No puedes vender ítems.", FontTypeNames_FONTTYPE_WARNING);
			return;
		}

		/* 'Is the other one dead?? */
		if (UserList[UserList[UserIndex].flags.TargetUser].flags.Muerto == 1) {
			WriteConsoleMsg(UserIndex, "¡¡No puedes comerciar con los muertos!!",
					FontTypeNames_FONTTYPE_INFO);
			return;
		}

		/* 'Is it me?? */
		if (UserList[UserIndex].flags.TargetUser == UserIndex) {
			WriteConsoleMsg(UserIndex, "¡¡No puedes comerciar con vos mismo!!", FontTypeNames_FONTTYPE_INFO);
			return;
		}

		/* 'Check distance */
		if (Distancia(UserList[UserList[UserIndex].flags.TargetUser].Pos, UserList[UserIndex].Pos) > 3) {
			WriteConsoleMsg(UserIndex, "Estás demasiado lejos del usuario.", FontTypeNames_FONTTYPE_INFO);
			return;
		}

		/* 'Is he already trading?? is it with me or someone else?? */
		if (UserList[UserList[UserIndex].flags.TargetUser].flags.Comerciando == true
				&& UserList[UserList[UserIndex].flags.TargetUser].ComUsu.DestUsu != UserIndex) {
			WriteConsoleMsg(UserIndex, "No puedes comerciar con el usuario en este momento.",
					FontTypeNames_FONTTYPE_INFO);
			return;
		}

		/* 'Initialize some variables... */
		UserList[UserIndex].ComUsu.DestUsu = UserList[UserIndex].flags.TargetUser;
		UserList[UserIndex].ComUsu.DestNick = UserList[UserList[UserIndex].flags.TargetUser].Name;
		for (i = (1); i <= (MAX_OFFER_SLOTS); i++) {
			UserList[UserIndex].ComUsu.cant[i] = 0;
			UserList[UserIndex].ComUsu.Objeto[i] = 0;
		}
		UserList[UserIndex].ComUsu.GoldAmount = 0;

		UserList[UserIndex].ComUsu.Acepto = false;
		UserList[UserIndex].ComUsu.Confirmo = false;

		/* 'Rutina para comerciar con otro usuario */
		IniciarComercioConUsuario(UserIndex, UserList[UserIndex].flags.TargetUser);
	} else {
		WriteConsoleMsg(UserIndex, "Primero haz click izquierdo sobre el personaje.",
				FontTypeNames_FONTTYPE_INFO);
	}
}

/* '' */
/* ' Handles the "BankStart" message. */
/* ' */
/* ' @param    userIndex The index of the user sending the message. */

void HandleBankStart(int UserIndex) {
	/* '*************************************************** */
	/* 'Author: Juan Martín Sotuyo Dodero (Maraxus) */
	/* 'Last Modification: 05/17/06 */
	/* ' */
	/* '*************************************************** */
	/* 'Remove packet ID */
	UserList[UserIndex].sockctx->incomingData->ReadByte();

	/* 'Dead people can't commerce */
	if (UserList[UserIndex].flags.Muerto == 1) {
		WriteConsoleMsg(UserIndex, "¡¡Estás muerto!!", FontTypeNames_FONTTYPE_INFO);
		return;
	}

	if (UserList[UserIndex].flags.Comerciando) {
		WriteConsoleMsg(UserIndex, "Ya estás comerciando.", FontTypeNames_FONTTYPE_INFO);
		return;
	}

	/* 'Validate target NPC */
	if (UserList[UserIndex].flags.TargetNPC > 0) {
		if (Distancia(Npclist[UserList[UserIndex].flags.TargetNPC].Pos, UserList[UserIndex].Pos) > 3) {
			WriteConsoleMsg(UserIndex, "Estás demasiado lejos del vendedor.", FontTypeNames_FONTTYPE_INFO);
			return;
		}

		/* 'If it's the banker.... */
		if (Npclist[UserList[UserIndex].flags.TargetNPC].NPCtype == eNPCType_Banquero) {
			IniciarDeposito(UserIndex);
		}
	} else {
		WriteConsoleMsg(UserIndex, "Primero haz click izquierdo sobre el personaje.",
				FontTypeNames_FONTTYPE_INFO);
	}
}

/* '' */
/* ' Handles the "Enlist" message. */
/* ' */
/* ' @param    userIndex The index of the user sending the message. */

void HandleEnlist(int UserIndex) {
	/* '*************************************************** */
	/* 'Author: Juan Martín Sotuyo Dodero (Maraxus) */
	/* 'Last Modification: 05/17/06 */
	/* ' */
	/* '*************************************************** */
	/* 'Remove packet ID */
	UserList[UserIndex].sockctx->incomingData->ReadByte();

	/* 'Validate target NPC */
	if (UserList[UserIndex].flags.TargetNPC == 0) {
		WriteConsoleMsg(UserIndex,
				"Primero tienes que seleccionar un personaje, haz click izquierdo sobre él.",
				FontTypeNames_FONTTYPE_INFO);
		return;
	}

	if (Npclist[UserList[UserIndex].flags.TargetNPC].NPCtype != eNPCType_Noble
			|| UserList[UserIndex].flags.Muerto != 0) {
		return;
	}

	if (Distancia(UserList[UserIndex].Pos, Npclist[UserList[UserIndex].flags.TargetNPC].Pos) > 4) {
		WriteConsoleMsg(UserIndex, "Debes acercarte más.", FontTypeNames_FONTTYPE_INFO);
		return;
	}

	if (Npclist[UserList[UserIndex].flags.TargetNPC].flags.Faccion == 0) {
		EnlistarArmadaReal(UserIndex);
	} else {
		EnlistarCaos(UserIndex);
	}
}

/* '' */
/* ' Handles the "Information" message. */
/* ' */
/* ' @param    userIndex The index of the user sending the message. */

void HandleInformation(int UserIndex) {
	/* '*************************************************** */
	/* 'Author: Juan Martín Sotuyo Dodero (Maraxus) */
	/* 'Last Modification: 05/17/06 */
	/* ' */
	/* '*************************************************** */
	int Matados;
	int NextRecom;
	int Diferencia;

	/* 'Remove packet ID */
	UserList[UserIndex].sockctx->incomingData->ReadByte();

	/* 'Validate target NPC */
	if (UserList[UserIndex].flags.TargetNPC == 0) {
		WriteConsoleMsg(UserIndex,
				"Primero tienes que seleccionar un personaje, haz click izquierdo sobre él.",
				FontTypeNames_FONTTYPE_INFO);
		return;
	}

	if (Npclist[UserList[UserIndex].flags.TargetNPC].NPCtype != eNPCType_Noble
			|| UserList[UserIndex].flags.Muerto != 0) {
		return;
	}

	if (Distancia(UserList[UserIndex].Pos, Npclist[UserList[UserIndex].flags.TargetNPC].Pos) > 4) {
		WriteConsoleMsg(UserIndex, "Estás demasiado lejos.", FontTypeNames_FONTTYPE_INFO);
		return;
	}

	NextRecom = UserList[UserIndex].Faccion.NextRecompensa;

	if (Npclist[UserList[UserIndex].flags.TargetNPC].flags.Faccion == 0) {
		if (UserList[UserIndex].Faccion.ArmadaReal == 0) {
			WriteChatOverHead(UserIndex, "¡¡No perteneces a las tropas reales!!",
					Npclist[UserList[UserIndex].flags.TargetNPC].Char.CharIndex, 0x00ffffff);
			return;
		}

		Matados = UserList[UserIndex].Faccion.CriminalesMatados;
		Diferencia = NextRecom - Matados;

		if (Diferencia > 0) {
			WriteChatOverHead(UserIndex,
					"Tu deber es combatir criminales, mata " + vb6::CStr(Diferencia)
							+ " criminales más y te daré una recompensa.",
					Npclist[UserList[UserIndex].flags.TargetNPC].Char.CharIndex, 0x00ffffff);
		} else {
			WriteChatOverHead(UserIndex,
					"Tu deber es combatir criminales, y ya has matado los suficientes como para merecerte una recompensa.",
					Npclist[UserList[UserIndex].flags.TargetNPC].Char.CharIndex, 0x00ffffff);
		}
	} else {
		if (UserList[UserIndex].Faccion.FuerzasCaos == 0) {
			WriteChatOverHead(UserIndex, "¡¡No perteneces a la legión oscura!!",
					Npclist[UserList[UserIndex].flags.TargetNPC].Char.CharIndex, 0x00ffffff);
			return;
		}

		Matados = UserList[UserIndex].Faccion.CiudadanosMatados;
		Diferencia = NextRecom - Matados;

		if (Diferencia > 0) {
			WriteChatOverHead(UserIndex,
					"Tu deber es sembrar el caos y la desesperanza, mata " + vb6::CStr(Diferencia)
							+ " ciudadanos más y te daré una recompensa.",
					Npclist[UserList[UserIndex].flags.TargetNPC].Char.CharIndex, 0x00ffffff);
		} else {
			WriteChatOverHead(UserIndex,
					"Tu deber es sembrar el caos y la desesperanza, y creo que estás en condiciones de merecer una recompensa.",
					Npclist[UserList[UserIndex].flags.TargetNPC].Char.CharIndex, 0x00ffffff);
		}
	}
}

/* '' */
/* ' Handles the "Reward" message. */
/* ' */
/* ' @param    userIndex The index of the user sending the message. */

void HandleReward(int UserIndex) {
	/* '*************************************************** */
	/* 'Author: Juan Martín Sotuyo Dodero (Maraxus) */
	/* 'Last Modification: 05/17/06 */
	/* ' */
	/* '*************************************************** */
	/* 'Remove packet ID */
	UserList[UserIndex].sockctx->incomingData->ReadByte();

	/* 'Validate target NPC */
	if (UserList[UserIndex].flags.TargetNPC == 0) {
		WriteConsoleMsg(UserIndex,
				"Primero tienes que seleccionar un personaje, haz click izquierdo sobre él.",
				FontTypeNames_FONTTYPE_INFO);
		return;
	}

	if (Npclist[UserList[UserIndex].flags.TargetNPC].NPCtype != eNPCType_Noble
			|| UserList[UserIndex].flags.Muerto != 0) {
		return;
	}

	if (Distancia(UserList[UserIndex].Pos, Npclist[UserList[UserIndex].flags.TargetNPC].Pos) > 4) {
		WriteConsoleMsg(UserIndex, "Estás demasiado lejos.", FontTypeNames_FONTTYPE_INFO);
		return;
	}

	if (Npclist[UserList[UserIndex].flags.TargetNPC].flags.Faccion == 0) {
		if (UserList[UserIndex].Faccion.ArmadaReal == 0) {
			WriteChatOverHead(UserIndex, "¡¡No perteneces a las tropas reales!!",
					Npclist[UserList[UserIndex].flags.TargetNPC].Char.CharIndex, 0x00ffffff);
			return;
		}
		RecompensaArmadaReal(UserIndex);
	} else {
		if (UserList[UserIndex].Faccion.FuerzasCaos == 0) {
			WriteChatOverHead(UserIndex, "¡¡No perteneces a la legión oscura!!",
					Npclist[UserList[UserIndex].flags.TargetNPC].Char.CharIndex, 0x00ffffff);
			return;
		}
		RecompensaCaos(UserIndex);
	}
}

/* '' */
/* ' Handles the "RequestMOTD" message. */
/* ' */
/* ' @param    userIndex The index of the user sending the message. */

void HandleRequestMOTD(int UserIndex) {
	/* '*************************************************** */
	/* 'Author: Juan Martín Sotuyo Dodero (Maraxus) */
	/* 'Last Modification: 05/17/06 */
	/* ' */
	/* '*************************************************** */
	/* 'Remove packet ID */
	UserList[UserIndex].sockctx->incomingData->ReadByte();

	SendMOTD(UserIndex);
}

/* '' */
/* ' Handles the "UpTime" message. */
/* ' */
/* ' @param    userIndex The index of the user sending the message. */

void HandleUpTime(int UserIndex) {
	/* '*************************************************** */
	/* 'Author: Juan Martín Sotuyo Dodero (Maraxus) */
	/* 'Last Modification: 01/10/08 */
	/* '01/10/2008 - Marcos Martinez (ByVal) - Automatic restart removed from the server along with all their assignments and varibles */
	/* '*************************************************** */
	/* 'Remove packet ID */
	UserList[UserIndex].sockctx->incomingData->ReadByte();

	int time;
	std::string UpTimeStr;

	/* 'Get total time in seconds */
	time = getInterval((vb6::GetTickCount()), tInicioServer) / 1000;

	/* 'Get times in dd:hh:mm:ss format */
	UpTimeStr = vb6::CStr(time % 60) + " segundos.";
	time = time / 60;

	UpTimeStr = vb6::CStr(time % 60) + " minutos, " + UpTimeStr;
	time = time / 60;

	UpTimeStr = vb6::CStr(time % 24) + " horas, " + UpTimeStr;
	time = time / 24;

	if (time == 1) {
		UpTimeStr = vb6::CStr(time) + " día, " + UpTimeStr;
	} else {
		UpTimeStr = vb6::CStr(time) + " días, " + UpTimeStr;
	}

	WriteConsoleMsg(UserIndex, "Server Online: " + UpTimeStr, FontTypeNames_FONTTYPE_INFO);
}

/* '' */
/* ' Handles the "PartyLeave" message. */
/* ' */
/* ' @param    userIndex The index of the user sending the message. */

void HandlePartyLeave(int UserIndex) {
	/* '*************************************************** */
	/* 'Author: Juan Martín Sotuyo Dodero (Maraxus) */
	/* 'Last Modification: 05/17/06 */
	/* ' */
	/* '*************************************************** */
	/* 'Remove packet ID */
	UserList[UserIndex].sockctx->incomingData->ReadByte();

	SalirDeParty(UserIndex);
}

/* '' */
/* ' Handles the "PartyCreate" message. */
/* ' */
/* ' @param    userIndex The index of the user sending the message. */

void HandlePartyCreate(int UserIndex) {
	/* '*************************************************** */
	/* 'Author: Juan Martín Sotuyo Dodero (Maraxus) */
	/* 'Last Modification: 05/17/06 */
	/* ' */
	/* '*************************************************** */
	/* 'Remove packet ID */
	UserList[UserIndex].sockctx->incomingData->ReadByte();

	if (!PuedeCrearParty(UserIndex)) {
		return;
	}

	CrearParty(UserIndex);
}

/* '' */
/* ' Handles the "PartyJoin" message. */
/* ' */
/* ' @param    userIndex The index of the user sending the message. */

void HandlePartyJoin(int UserIndex) {
	/* '*************************************************** */
	/* 'Author: Juan Martín Sotuyo Dodero (Maraxus) */
	/* 'Last Modification: 05/17/06 */
	/* ' */
	/* '*************************************************** */
	/* 'Remove packet ID */
	UserList[UserIndex].sockctx->incomingData->ReadByte();

	SolicitarIngresoAParty(UserIndex);
}

/* '' */
/* ' Handles the "ShareNpc" message. */
/* ' */
/* ' @param    userIndex The index of the user sending the message. */

void HandleShareNpc(int UserIndex) {
	/* '*************************************************** */
	/* 'Author: ZaMa */
	/* 'Last Modification: 15/04/2010 */
	/* 'Shares owned npcs with other user */
	/* '*************************************************** */

	int TargetUserIndex;
	int SharingUserIndex;

	/* 'Remove packet ID */
	UserList[UserIndex].sockctx->incomingData->ReadByte();

	/* ' Didn't target any user */
	TargetUserIndex = UserList[UserIndex].flags.TargetUser;
	if (TargetUserIndex == 0) {
		return;
	}

	/* ' Can't share with admins */
	if (EsGm(TargetUserIndex)) {
		WriteConsoleMsg(UserIndex, "No puedes compartir npcs con administradores!!",
				FontTypeNames_FONTTYPE_INFO);
		return;
	}

	/* ' Pk or Caos? */
	if (criminal(UserIndex)) {
		/* ' Caos can only share with other caos */
		if (esCaos(UserIndex)) {
			if (!esCaos(TargetUserIndex)) {
				WriteConsoleMsg(UserIndex, "Solo puedes compartir npcs con miembros de tu misma facción!!",
						FontTypeNames_FONTTYPE_INFO);
				return;
			}

			/* ' Pks don't need to share with anyone */
		} else {
			return;
		}

		/* ' Ciuda or Army? */
	} else {
		/* ' Can't share */
		if (criminal(TargetUserIndex)) {
			WriteConsoleMsg(UserIndex, "No puedes compartir npcs con criminales!!",
					FontTypeNames_FONTTYPE_INFO);
			return;
		}
	}

	/* ' Already sharing with target */
	SharingUserIndex = UserList[UserIndex].flags.ShareNpcWith;
	if (SharingUserIndex == TargetUserIndex) {
		return;
	}

	/* ' Aviso al usuario anterior que dejo de compartir */
	if (SharingUserIndex != 0) {
		WriteConsoleMsg(SharingUserIndex,
				UserList[UserIndex].Name + " ha dejado de compartir sus npcs contigo.",
				FontTypeNames_FONTTYPE_INFO);
		WriteConsoleMsg(UserIndex,
				"Has dejado de compartir tus npcs con " + UserList[SharingUserIndex].Name + ".",
				FontTypeNames_FONTTYPE_INFO);
	}

	UserList[UserIndex].flags.ShareNpcWith = TargetUserIndex;

	WriteConsoleMsg(TargetUserIndex, UserList[UserIndex].Name + " ahora comparte sus npcs contigo.",
			FontTypeNames_FONTTYPE_INFO);
	WriteConsoleMsg(UserIndex, "Ahora compartes tus npcs con " + UserList[TargetUserIndex].Name + ".",
			FontTypeNames_FONTTYPE_INFO);

}

/* '' */
/* ' Handles the "StopSharingNpc" message. */
/* ' */
/* ' @param    userIndex The index of the user sending the message. */

void HandleStopSharingNpc(int UserIndex) {
	/* '*************************************************** */
	/* 'Author: ZaMa */
	/* 'Last Modification: 15/04/2010 */
	/* 'Stop Sharing owned npcs with other user */
	/* '*************************************************** */

	int SharingUserIndex;

	/* 'Remove packet ID */
	UserList[UserIndex].sockctx->incomingData->ReadByte();

	SharingUserIndex = UserList[UserIndex].flags.ShareNpcWith;

	if (SharingUserIndex != 0) {

		/* ' Aviso al que compartia y al que le compartia. */
		WriteConsoleMsg(SharingUserIndex,
				UserList[UserIndex].Name + " ha dejado de compartir sus npcs contigo.",
				FontTypeNames_FONTTYPE_INFO);
		WriteConsoleMsg(SharingUserIndex,
				"Has dejado de compartir tus npcs con " + UserList[SharingUserIndex].Name + ".",
				FontTypeNames_FONTTYPE_INFO);

		UserList[UserIndex].flags.ShareNpcWith = 0;
	}

}

/* '' */
/* ' Handles the "Inquiry" message. */
/* ' */
/* ' @param    userIndex The index of the user sending the message. */

void HandleInquiry(int UserIndex) {
	/* '*************************************************** */
	/* 'Author: Juan Martín Sotuyo Dodero (Maraxus) */
	/* 'Last Modification: 05/17/06 */
	/* ' */
	/* '*************************************************** */
	/* 'Remove packet ID */
	UserList[UserIndex].sockctx->incomingData->ReadByte();

	ConsultaPopular->SendInfoEncuesta(UserIndex);
}

/* '' */
/* ' Handles the "GuildMessage" message. */
/* ' */
/* ' @param    userIndex The index of the user sending the message. */

void HandleGuildMessage(int UserIndex) {
	/* '*************************************************** */
	/* 'Author: Juan Martín Sotuyo Dodero (Maraxus) */
	/* 'Last Modification: 15/07/2009 */
	/* '02/03/2009: ZaMa - Arreglado un indice mal pasado a la funcion de cartel de clanes overhead. */
	/* '15/07/2009: ZaMa - Now invisible admins only speak by console */
	/* '*************************************************** */
	if (UserList[UserIndex].sockctx->incomingData->length() < 3) {
		throw insufficient_data_error();
		return;
	}

	/* 'This packet contains strings, make a copy of the data to prevent losses if it's not complete yet... */
	clsByteQueue* buffer = UserList[UserIndex].sockctx->incomingData.get();


	/* 'Remove packet ID */
	buffer->ReadByte();

	std::string Chat;

	Chat = buffer->ReadUnicodeString();

	if (vb6::LenB(Chat) != 0) {
		/* 'Analize chat... */
		ParseChat(Chat);

		if (UserList[UserIndex].GuildIndex > 0) {
			SendData(SendTarget_ToDiosesYclan, UserList[UserIndex].GuildIndex,
					PrepareMessageGuildChat(UserList[UserIndex].Name + "> " + Chat));

			if (!(UserList[UserIndex].flags.AdminInvisible == 1)) {
				SendData(SendTarget_ToClanArea, UserIndex,
						PrepareMessageChatOverHead("< " + Chat + " >", UserList[UserIndex].Char.CharIndex,
								vbYellow));
			}
		}
	}

	/* 'If we got here then packet is complete, copy data back to original queue */


}

/* '' */
/* ' Handles the "PartyMessage" message. */
/* ' */
/* ' @param    userIndex The index of the user sending the message. */

void HandlePartyMessage(int UserIndex) {
	/* '*************************************************** */
	/* 'Author: Juan Martín Sotuyo Dodero (Maraxus) */
	/* 'Last Modification: 05/17/06 */
	/* ' */
	/* '*************************************************** */
	if (UserList[UserIndex].sockctx->incomingData->length() < 3) {
		throw insufficient_data_error();
		return;
	}

	/* 'This packet contains strings, make a copy of the data to prevent losses if it's not complete yet... */
	clsByteQueue* buffer = UserList[UserIndex].sockctx->incomingData.get();


	/* 'Remove packet ID */
	buffer->ReadByte();

	std::string Chat;

	Chat = buffer->ReadUnicodeString();

	if (vb6::LenB(Chat) != 0) {
		/* 'Analize chat... */
		ParseChat(Chat);

		BroadCastParty(UserIndex, Chat);
		/* 'TODO : Con la 0.12.1 se debe definir si esto vuelve o se borra (/CMSG overhead) */
		/* 'Call SendData(SendTarget.ToPartyArea, UserIndex, UserList(UserIndex).Pos.map, "||" & vbYellow & "°< " & mid$(rData, 7) & " >°" & CStr(UserList(UserIndex).Char.CharIndex)) */
	}

	/* 'If we got here then packet is complete, copy data back to original queue */


}

/* '' */
/* ' Handles the "CentinelReport" message. */
/* ' */
/* ' @param    userIndex The index of the user sending the message. */

void HandleCentinelReport(int UserIndex) {
	/* '*************************************************** */
	/* 'Author: Juan Martín Sotuyo Dodero (Maraxus) */
	/* 'Last Modification: 05/17/06 */
	/* ' */
	/* '*************************************************** */
	if (UserList[UserIndex].sockctx->incomingData->length() < 3) {
		throw insufficient_data_error();
		return;
	}

	/* 'Remove packet ID */
	UserList[UserIndex].sockctx->incomingData->ReadByte();

	CentinelaCheckClave(UserIndex, UserList[UserIndex].sockctx->incomingData->ReadInteger());
}

/* '' */
/* ' Handles the "GuildOnline" message. */
/* ' */
/* ' @param    userIndex The index of the user sending the message. */

void HandleGuildOnline(int UserIndex) {
	/* '*************************************************** */
	/* 'Author: Juan Martín Sotuyo Dodero (Maraxus) */
	/* 'Last Modification: 05/17/06 */
	/* ' */
	/* '*************************************************** */
	/* 'Remove packet ID */
	UserList[UserIndex].sockctx->incomingData->ReadByte();

	std::string onlineList;

	onlineList = m_ListaDeMiembrosOnline(UserIndex, UserList[UserIndex].GuildIndex);

	if (UserList[UserIndex].GuildIndex != 0) {
		WriteConsoleMsg(UserIndex, "Companeros de tu clan conectados: " + onlineList,
				FontTypeNames_FONTTYPE_GUILDMSG);
	} else {
		WriteConsoleMsg(UserIndex, "No pertences a ningún clan.", FontTypeNames_FONTTYPE_GUILDMSG);
	}
}

/* '' */
/* ' Handles the "PartyOnline" message. */
/* ' */
/* ' @param    userIndex The index of the user sending the message. */

void HandlePartyOnline(int UserIndex) {
	/* '*************************************************** */
	/* 'Author: Juan Martín Sotuyo Dodero (Maraxus) */
	/* 'Last Modification: 05/17/06 */
	/* ' */
	/* '*************************************************** */
	/* 'Remove packet ID */
	UserList[UserIndex].sockctx->incomingData->ReadByte();

	OnlineParty(UserIndex);
}

/* '' */
/* ' Handles the "CouncilMessage" message. */
/* ' */
/* ' @param    userIndex The index of the user sending the message. */

void HandleCouncilMessage(int UserIndex) {
	/* '*************************************************** */
	/* 'Author: Juan Martín Sotuyo Dodero (Maraxus) */
	/* 'Last Modification: 05/17/06 */
	/* ' */
	/* '*************************************************** */
	if (UserList[UserIndex].sockctx->incomingData->length() < 3) {
		throw insufficient_data_error();
		return;
	}

	/* 'This packet contains strings, make a copy of the data to prevent losses if it's not complete yet... */
	clsByteQueue* buffer = UserList[UserIndex].sockctx->incomingData.get();


	/* 'Remove packet ID */
	buffer->ReadByte();

	std::string Chat;

	Chat = buffer->ReadUnicodeString();

	if (vb6::LenB(Chat) != 0) {
		/* 'Analize chat... */
		ParseChat(Chat);

		if (UserTieneAlgunPrivilegios(UserIndex, PlayerType_RoyalCouncil)) {
			SendData(SendTarget_ToConsejo, UserIndex,
					PrepareMessageConsoleMsg("(Consejero) " + UserList[UserIndex].Name + "> " + Chat,
							FontTypeNames_FONTTYPE_CONSEJO));
		} else if (UserTieneAlgunPrivilegios(UserIndex, PlayerType_ChaosCouncil)) {
			SendData(SendTarget_ToConsejoCaos, UserIndex,
					PrepareMessageConsoleMsg("(Consejero) " + UserList[UserIndex].Name + "> " + Chat,
							FontTypeNames_FONTTYPE_CONSEJOCAOS));
		}
	}

	/* 'If we got here then packet is complete, copy data back to original queue */

}

/* '' */
/* ' Handles the "RoleMasterRequest" message. */
/* ' */
/* ' @param    userIndex The index of the user sending the message. */

void HandleRoleMasterRequest(int UserIndex) {
	/* '*************************************************** */
	/* 'Author: Juan Martín Sotuyo Dodero (Maraxus) */
	/* 'Last Modification: 05/17/06 */
	/* ' */
	/* '*************************************************** */
	if (UserList[UserIndex].sockctx->incomingData->length() < 3) {
		throw insufficient_data_error();
		return;
	}

	/* 'This packet contains strings, make a copy of the data to prevent losses if it's not complete yet... */
	clsByteQueue* buffer = UserList[UserIndex].sockctx->incomingData.get();


	/* 'Remove packet ID */
	buffer->ReadByte();

	std::string request;

	request = buffer->ReadUnicodeString();

	if (vb6::LenB(request) != 0) {
		WriteConsoleMsg(UserIndex, "Su solicitud ha sido enviada.", FontTypeNames_FONTTYPE_INFO);
		SendData(SendTarget_ToRMsAndHigherAdmins, 0,
				PrepareMessageConsoleMsg(UserList[UserIndex].Name + " PREGUNTA ROL: " + request,
						FontTypeNames_FONTTYPE_GUILDMSG));
	}

	/* 'If we got here then packet is complete, copy data back to original queue */

}

/* '' */
/* ' Handles the "GMRequest" message. */
/* ' */
/* ' @param    userIndex The index of the user sending the message. */

void HandleGMRequest(int UserIndex) {
	/* '*************************************************** */
	/* 'Author: Juan Martín Sotuyo Dodero (Maraxus) */
	/* 'Last Modification: 05/17/06 */
	/* ' */
	/* '*************************************************** */
	/* 'Remove packet ID */
	UserList[UserIndex].sockctx->incomingData->ReadByte();

	auto it = std::find(Ayuda.begin(), Ayuda.end(), UserList[UserIndex].Name);
	if (it == Ayuda.end()) {
		WriteConsoleMsg(UserIndex,
				"El mensaje ha sido entregado, ahora sólo debes esperar que se desocupe algún GM.",
				FontTypeNames_FONTTYPE_INFO);
		Ayuda.push_back(UserList[UserIndex].Name);
	} else {
		Ayuda.erase(it);
		Ayuda.push_back(UserList[UserIndex].Name);
		WriteConsoleMsg(UserIndex,
				"Ya habías mandado un mensaje, tu mensaje ha sido movido al final de la cola de mensajes.",
				FontTypeNames_FONTTYPE_INFO);
	}
}

/* '' */
/* ' Handles the "BugReport" message. */
/* ' */
/* ' @param    userIndex The index of the user sending the message. */

void HandleBugReport(int UserIndex) {
	/* '*************************************************** */
	/* 'Author: Juan Martín Sotuyo Dodero (Maraxus) */
	/* 'Last Modification: 05/17/06 */
	/* ' */
	/* '*************************************************** */
	if (UserList[UserIndex].sockctx->incomingData->length() < 3) {
		throw insufficient_data_error();
		return;
	}

	/* 'This packet contains strings, make a copy of the data to prevent losses if it's not complete yet... */
	clsByteQueue* buffer = UserList[UserIndex].sockctx->incomingData.get();

	/* 'Remove packet ID */
	buffer->ReadByte();

	std::string bugReport;

	bugReport = buffer->ReadUnicodeString();

	LogBugReport(UserIndex, bugReport);

	/* 'If we got here then packet is complete, copy data back to original queue */


}

/* '' */
/* ' Handles the "ChangeDescription" message. */
/* ' */
/* ' @param    userIndex The index of the user sending the message. */

void HandleChangeDescription(int UserIndex) {
	/* '*************************************************** */
	/* 'Author: Juan Martín Sotuyo Dodero (Maraxus) */
	/* 'Last Modification: 05/17/06 */
	/* ' */
	/* '*************************************************** */
	if (UserList[UserIndex].sockctx->incomingData->length() < 3) {
		throw insufficient_data_error();
		return;
	}

	/* 'This packet contains strings, make a copy of the data to prevent losses if it's not complete yet... */
	clsByteQueue* buffer = UserList[UserIndex].sockctx->incomingData.get();


	/* 'Remove packet ID */
	buffer->ReadByte();

	std::string description;

	description = buffer->ReadUnicodeString();

	if (UserList[UserIndex].flags.Muerto == 1) {
		WriteConsoleMsg(UserIndex, "No puedes cambiar la descripción estando muerto.",
				FontTypeNames_FONTTYPE_INFO);
	} else {
		if (!AsciiValidos(description)) {
			WriteConsoleMsg(UserIndex, "La descripción tiene caracteres inválidos.",
					FontTypeNames_FONTTYPE_INFO);
		} else {
			UserList[UserIndex].desc = vb6::Trim(description);
			WriteConsoleMsg(UserIndex, "La descripción ha cambiado.", FontTypeNames_FONTTYPE_INFO);
		}
	}

	/* 'If we got here then packet is complete, copy data back to original queue */


}

/* '' */
/* ' Handles the "GuildVote" message. */
/* ' */
/* ' @param    userIndex The index of the user sending the message. */

void HandleGuildVote(int UserIndex) {
	/* '*************************************************** */
	/* 'Author: Juan Martín Sotuyo Dodero (Maraxus) */
	/* 'Last Modification: 05/17/06 */
	/* ' */
	/* '*************************************************** */
	if (UserList[UserIndex].sockctx->incomingData->length() < 3) {
		throw insufficient_data_error();
		return;
	}

	/* 'This packet contains strings, make a copy of the data to prevent losses if it's not complete yet... */
	clsByteQueue* buffer = UserList[UserIndex].sockctx->incomingData.get();


	/* 'Remove packet ID */
	buffer->ReadByte();

	std::string vote;
	std::string errorStr;

	vote = buffer->ReadUnicodeString();

	if (!v_UsuarioVota(UserIndex, vote, errorStr)) {
		WriteConsoleMsg(UserIndex, "Voto NO contabilizado: " + errorStr, FontTypeNames_FONTTYPE_GUILD);
	} else {
		WriteConsoleMsg(UserIndex, "Voto contabilizado.", FontTypeNames_FONTTYPE_GUILD);
	}

	/* 'If we got here then packet is complete, copy data back to original queue */


}

/* '' */
/* ' Handles the "ShowGuildNews" message. */
/* ' */
/* ' @param    userIndex The index of the user sending the message. */

void HandleShowGuildNews(int UserIndex) {
	/* '*************************************************** */
	/* 'Author: ZaMA */
	/* 'Last Modification: 05/17/06 */
	/* ' */
	/* '*************************************************** */

	/* 'Remove packet ID */
	UserList[UserIndex].sockctx->incomingData->ReadByte();

	SendGuildNews(UserIndex);
}

/* '' */
/* ' Handles the "Punishments" message. */
/* ' */
/* ' @param    userIndex The index of the user sending the message. */

void HandlePunishments(int UserIndex) {
	/* '*************************************************** */
	/* 'Author: Juan Martín Sotuyo Dodero (Maraxus) */
	/* 'Last Modification: 25/08/2009 */
	/* '25/08/2009: ZaMa - Now only admins can see other admins' punishment list */
	/* '*************************************************** */
	if (UserList[UserIndex].sockctx->incomingData->length() < 3) {
		throw insufficient_data_error();
		return;
	}

	/* 'This packet contains strings, make a copy of the data to prevent losses if it's not complete yet... */
	clsByteQueue* buffer = UserList[UserIndex].sockctx->incomingData.get();


	/* 'Remove packet ID */
	buffer->ReadByte();

	std::string Name;
	int Count;

	Name = buffer->ReadUnicodeString();

	if (UserTieneAlgunPrivilegios(UserIndex, PlayerType_User)) {
		return;
	}

	if (vb6::LenB(Name) != 0) {
		if ((vb6::InStrB(Name, "/") != 0)) {
			Name = vb6::Replace(Name, "/", "");
		}
		if ((vb6::InStrB(Name, "/") != 0)) {
			Name = vb6::Replace(Name, "/", "");
		}
		if ((vb6::InStrB(Name, ":") != 0)) {
			Name = vb6::Replace(Name, ":", "");
		}
		if ((vb6::InStrB(Name, "|") != 0)) {
			Name = vb6::Replace(Name, "|", "");
		}

		if ((EsAdmin(Name) || EsDios(Name) || EsSemiDios(Name) || EsConsejero(Name) || EsRolesMaster(Name))
				&& (UserTieneAlgunPrivilegios(UserIndex, PlayerType_User))) {
			WriteConsoleMsg(UserIndex, "No puedes ver las penas de los administradores.",
					FontTypeNames_FONTTYPE_INFO);
		} else {
			if (FileExist(GetCharPath(Name), 0)) {
				Count = vb6::val(GetVar(GetCharPath(Name), "PENAS", "Cant"));
				if (Count == 0) {
					WriteConsoleMsg(UserIndex, "Sin prontuario..", FontTypeNames_FONTTYPE_INFO);
				} else {
					while (Count > 0) {
						WriteConsoleMsg(UserIndex,
								vb6::CStr(Count) + " - " + GetVar(GetCharPath(Name), "PENAS", "P" + vb6::CStr(Count)),
								FontTypeNames_FONTTYPE_INFO);
						Count = Count - 1;
					}
				}
			} else {
				WriteConsoleMsg(UserIndex, "Personaje " "" + Name + "" " inexistente.",
						FontTypeNames_FONTTYPE_INFO);
			}
		}
	}
}

/* '' */
/* ' Handles the "ChangePassword" message. */
/* ' */
/* ' @param    userIndex The index of the user sending the message. */

void HandleChangePassword(int UserIndex) {
	/* '*************************************************** */
	/* 'Author: Juan Martín Sotuyo Dodero (Maraxus) */
	/* 'Creation Date: 10/10/07 */
	/* 'Last Modified By: Rapsodius */
	/* '*************************************************** */
	if (UserList[UserIndex].sockctx->incomingData->length() < 5) {
		throw insufficient_data_error();
		return;
	}

	/* 'This packet contains strings, make a copy of the data to prevent losses if it's not complete yet... */
	clsByteQueue* buffer = UserList[UserIndex].sockctx->incomingData.get();


	std::string oldPass;
	std::string newPass;
	std::string oldPass2;

	/* 'Remove packet ID */
	buffer->ReadByte();

	/* # IF SeguridadAlkon THEN */
	/* # ELSE */
	oldPass = (buffer->ReadUnicodeString());
	newPass = (buffer->ReadUnicodeString());
	/* # END IF */

	if (vb6::LenB(newPass) == 0) {
		WriteConsoleMsg(UserIndex, "Debes especificar una contrasena nueva, inténtalo de nuevo.",
				FontTypeNames_FONTTYPE_INFO);
	} else {
		if (!CheckPasswordUser(UserList[UserIndex].Name, oldPass)) {
			WriteConsoleMsg(UserIndex,
					"La contrasena actual proporcionada no es correcta. La contrasena no ha sido cambiada, inténtalo de nuevo.",
					FontTypeNames_FONTTYPE_INFO);
		} else {
			// WriteVar(GetCharPath(UserList[UserIndex].Name), "INIT", "Password", newPass);
			WriteSaltedPasswordUser(UserList[UserIndex].Name, newPass);
			WriteConsoleMsg(UserIndex, "La contrasena fue cambiada con éxito.", FontTypeNames_FONTTYPE_INFO);
		}
	}

	/* 'If we got here then packet is complete, copy data back to original queue */


}

/* '' */
/* ' Handles the "Gamble" message. */
/* ' */
/* ' @param    userIndex The index of the user sending the message. */

void HandleGamble(int UserIndex) {
	/* '*************************************************** */
	/* 'Author: Juan Martín Sotuyo Dodero (Maraxus) */
	/* 'Last Modification: 05/17/06 */
	/* '10/07/2010: ZaMa - Now normal npcs don't answer if asked to gamble. */
	/* '*************************************************** */
	if (UserList[UserIndex].sockctx->incomingData->length() < 3) {
		throw insufficient_data_error();
		return;
	}

	/* 'Remove packet ID */
	UserList[UserIndex].sockctx->incomingData->ReadByte();

	int Amount;

	Amount = UserList[UserIndex].sockctx->incomingData->ReadInteger();

	/* ' Dead? */
	if (UserList[UserIndex].flags.Muerto == 1) {
		WriteConsoleMsg(UserIndex, "¡¡Estás muerto!!", FontTypeNames_FONTTYPE_INFO);

		/* 'Validate target NPC */
	} else if (UserList[UserIndex].flags.TargetNPC == 0) {
		WriteConsoleMsg(UserIndex,
				"Primero tienes que seleccionar un personaje, haz click izquierdo sobre él.",
				FontTypeNames_FONTTYPE_INFO);

		/* ' Validate Distance */
	} else if (Distancia(Npclist[UserList[UserIndex].flags.TargetNPC].Pos, UserList[UserIndex].Pos) > 10) {
		WriteConsoleMsg(UserIndex, "Estás demasiado lejos.", FontTypeNames_FONTTYPE_INFO);

		/* ' Validate NpcType */
	} else if (Npclist[UserList[UserIndex].flags.TargetNPC].NPCtype != eNPCType_Timbero) {

		eNPCType TargetNpcType;
		TargetNpcType = Npclist[UserList[UserIndex].flags.TargetNPC].NPCtype;

		/* ' Normal npcs don't speak */
		if (TargetNpcType != eNPCType_Comun && TargetNpcType != eNPCType_DRAGON
				&& TargetNpcType != eNPCType_Pretoriano) {
			WriteChatOverHead(UserIndex, "No tengo ningún interés en apostar.",
					Npclist[UserList[UserIndex].flags.TargetNPC].Char.CharIndex, 0x00ffffff);
		}

		/* ' Validate amount */
	} else if (Amount < 1) {
		WriteChatOverHead(UserIndex, "El mínimo de apuesta es 1 moneda.",
				Npclist[UserList[UserIndex].flags.TargetNPC].Char.CharIndex, 0x00ffffff);

		/* ' Validate amount */
	} else if (Amount > 5000) {
		WriteChatOverHead(UserIndex, "El máximo de apuesta es 5000 monedas.",
				Npclist[UserList[UserIndex].flags.TargetNPC].Char.CharIndex, 0x00ffffff);

		/* ' Validate user gold */
	} else if (UserList[UserIndex].Stats.GLD < Amount) {
		WriteChatOverHead(UserIndex, "No tienes esa cantidad.",
				Npclist[UserList[UserIndex].flags.TargetNPC].Char.CharIndex, 0x00ffffff);

	} else {
		if (RandomNumber(1, 100) <= 47) {
			UserList[UserIndex].Stats.GLD = UserList[UserIndex].Stats.GLD + Amount;
			WriteChatOverHead(UserIndex, "¡Felicidades! Has ganado " + vb6::CStr(Amount) + " monedas de oro.",
					Npclist[UserList[UserIndex].flags.TargetNPC].Char.CharIndex, 0x00ffffff);

			Apuestas.Perdidas = Apuestas.Perdidas + Amount;
			WriteVar(GetDatPath(DATPATH::apuestas), "Main", "Perdidas", vb6::CStr(Apuestas.Perdidas));
		} else {
			UserList[UserIndex].Stats.GLD = UserList[UserIndex].Stats.GLD - Amount;
			WriteChatOverHead(UserIndex, "Lo siento, has perdido " + vb6::CStr(Amount) + " monedas de oro.",
					Npclist[UserList[UserIndex].flags.TargetNPC].Char.CharIndex, 0x00ffffff);

			Apuestas.Ganancias = Apuestas.Ganancias + Amount;
			WriteVar(GetDatPath(DATPATH::apuestas), "Main", "Ganancias", vb6::CStr(Apuestas.Ganancias));
		}

		Apuestas.Jugadas = Apuestas.Jugadas + 1;

		WriteVar(GetDatPath(DATPATH::apuestas), "Main", "Jugadas", vb6::CStr(Apuestas.Jugadas));

		WriteUpdateGold(UserIndex);
	}
}

/* '' */
/* ' Handles the "InquiryVote" message. */
/* ' */
/* ' @param    userIndex The index of the user sending the message. */

void HandleInquiryVote(int UserIndex) {
	/* '*************************************************** */
	/* 'Author: Juan Martín Sotuyo Dodero (Maraxus) */
	/* 'Last Modification: 05/17/06 */
	/* ' */
	/* '*************************************************** */
	if (UserList[UserIndex].sockctx->incomingData->length() < 2) {
		throw insufficient_data_error();
		return;
	}

	/* 'Remove packet ID */
	UserList[UserIndex].sockctx->incomingData->ReadByte();

	int opt;

	opt = UserList[UserIndex].sockctx->incomingData->ReadByte();

	WriteConsoleMsg(UserIndex, ConsultaPopular->doVotar(UserIndex, opt), FontTypeNames_FONTTYPE_GUILD);
}

/* '' */
/* ' Handles the "BankExtractGold" message. */
/* ' */
/* ' @param    userIndex The index of the user sending the message. */

void HandleBankExtractGold(int UserIndex) {
	/* '*************************************************** */
	/* 'Author: Juan Martín Sotuyo Dodero (Maraxus) */
	/* 'Last Modification: 05/17/06 */
	/* ' */
	/* '*************************************************** */
	if (UserList[UserIndex].sockctx->incomingData->length() < 5) {
		throw insufficient_data_error();
		return;
	}

	/* 'Remove packet ID */
	UserList[UserIndex].sockctx->incomingData->ReadByte();

	int Amount;

	Amount = UserList[UserIndex].sockctx->incomingData->ReadLong();

	/* 'Dead people can't leave a faction.. they can't talk... */
	if (UserList[UserIndex].flags.Muerto == 1) {
		WriteConsoleMsg(UserIndex, "¡¡Estás muerto!!", FontTypeNames_FONTTYPE_INFO);
		return;
	}

	/* 'Validate target NPC */
	if (UserList[UserIndex].flags.TargetNPC == 0) {
		WriteConsoleMsg(UserIndex,
				"Primero tienes que seleccionar un personaje, haz click izquierdo sobre él.",
				FontTypeNames_FONTTYPE_INFO);
		return;
	}

	if (Npclist[UserList[UserIndex].flags.TargetNPC].NPCtype != eNPCType_Banquero) {
		return;
	}

	if (Distancia(UserList[UserIndex].Pos, Npclist[UserList[UserIndex].flags.TargetNPC].Pos) > 10) {
		WriteConsoleMsg(UserIndex, "Estás demasiado lejos.", FontTypeNames_FONTTYPE_INFO);
		return;
	}

	if (Amount > 0 && Amount <= UserList[UserIndex].Stats.Banco) {
		UserList[UserIndex].Stats.Banco = UserList[UserIndex].Stats.Banco - Amount;
		UserList[UserIndex].Stats.GLD = UserList[UserIndex].Stats.GLD + Amount;
		WriteChatOverHead(UserIndex,
				"Tenés " + vb6::CStr(UserList[UserIndex].Stats.Banco) + " monedas de oro en tu cuenta.",
				Npclist[UserList[UserIndex].flags.TargetNPC].Char.CharIndex, 0x00ffffff);
	} else {
		WriteChatOverHead(UserIndex, "No tienes esa cantidad.",
				Npclist[UserList[UserIndex].flags.TargetNPC].Char.CharIndex, 0x00ffffff);
	}

	WriteUpdateGold(UserIndex);
	WriteUpdateBankGold(UserIndex);
}

/* '' */
/* ' Handles the "LeaveFaction" message. */
/* ' */
/* ' @param    userIndex The index of the user sending the message. */

void HandleLeaveFaction(int UserIndex) {
	/* '*************************************************** */
	/* 'Author: Juan Martín Sotuyo Dodero (Maraxus) */
	/* 'Last Modification: 09/28/2010 */
	/* ' 09/28/2010 C4b3z0n - Ahora la respuesta de los NPCs sino perteneces a ninguna facción solo la hacen el Rey o el Demonio */
	/* ' 05/17/06 - Maraxus */
	/* '*************************************************** */

	bool TalkToKing = false;
	bool TalkToDemon = false;
	int NpcIndex = 0;

	/* 'Remove packet ID */
	UserList[UserIndex].sockctx->incomingData->ReadByte();

	/* 'Dead people can't leave a faction.. they can't talk... */
	if (UserList[UserIndex].flags.Muerto == 1) {
		WriteConsoleMsg(UserIndex, "¡¡Estás muerto!!", FontTypeNames_FONTTYPE_INFO);
		return;
	}

	/* ' Chequea si habla con el rey o el demonio. Puede salir sin hacerlo, pero si lo hace le reponden los npcs */
	NpcIndex = UserList[UserIndex].flags.TargetNPC;
	if (NpcIndex != 0) {
		/* ' Es rey o domonio? */
		if (Npclist[NpcIndex].NPCtype == eNPCType_Noble) {
			/* 'Rey? */
			if (Npclist[NpcIndex].flags.Faccion == 0) {
				TalkToKing = true;
				/* ' Demonio */
			} else {
				TalkToDemon = true;
			}
		}
	}

	/* 'Quit the Royal Army? */
	if (UserList[UserIndex].Faccion.ArmadaReal == 1) {
		/* ' Si le pidio al demonio salir de la armada, este le responde. */
		if (TalkToDemon) {
			if (NpcIndex > 0) {
				WriteChatOverHead(UserIndex, "¡¡¡Sal de aquí bufón!!!", Npclist[NpcIndex].Char.CharIndex,
					0x00ffffff);
			}

		} else {
			/* ' Si le pidio al rey salir de la armada, le responde. */
			if (TalkToKing) {
				if (NpcIndex > 0) {
					WriteChatOverHead(UserIndex, "Serás bienvenido a las fuerzas imperiales si deseas regresar.",
						Npclist[NpcIndex].Char.CharIndex, 0x00ffffff);
				}
			}

			ExpulsarFaccionReal(UserIndex, false);

		}

		/* 'Quit the Chaos Legion? */
	} else if (UserList[UserIndex].Faccion.FuerzasCaos == 1) {
		/* ' Si le pidio al rey salir del caos, le responde. */
		if (TalkToKing) {
			if (NpcIndex > 0) {
				WriteChatOverHead(UserIndex, "¡¡¡Sal de aquí maldito criminal!!!",
						Npclist[NpcIndex].Char.CharIndex, 0x00ffffff);
			}
		} else {
			/* ' Si le pidio al demonio salir del caos, este le responde. */
			if (TalkToDemon) {
				if (NpcIndex > 0) {
					WriteChatOverHead(UserIndex, "Ya volverás arrastrandote.", Npclist[NpcIndex].Char.CharIndex,
							0x00ffffff);
				}
			}

			ExpulsarFaccionCaos(UserIndex, false);
		}
		/* ' No es faccionario */
	} else {

		/* ' Si le hablaba al rey o demonio, le repsonden ellos */
		/* 'Corregido, solo si son en efecto el rey o el demonio, no cualquier NPC (C4b3z0n) */
		/* 'Si se pueden unir a la facción (status), son invitados */
		if ((TalkToDemon && criminal(UserIndex)) || (TalkToKing && !criminal(UserIndex))) {
			if (NpcIndex > 0) {
				WriteChatOverHead(UserIndex, "No perteneces a nuestra facción. Si deseas unirte, di /ENLISTAR",
						Npclist[NpcIndex].Char.CharIndex, 0x00ffffff);
			}
		} else if ((TalkToDemon && !criminal(UserIndex))) {
			if (NpcIndex > 0) {
				WriteChatOverHead(UserIndex, "¡¡¡Sal de aquí bufón!!!", Npclist[NpcIndex].Char.CharIndex,
						0x00ffffff);
			}
		} else if ((TalkToKing && criminal(UserIndex))) {
			if (NpcIndex > 0) {
				WriteChatOverHead(UserIndex, "¡¡¡Sal de aquí maldito criminal!!!",
						Npclist[NpcIndex].Char.CharIndex, 0x00ffffff);
			}
		} else {
			WriteConsoleMsg(UserIndex, "¡No perteneces a ninguna facción!", FontTypeNames_FONTTYPE_FIGHT);
		}

	}

}

/* '' */
/* ' Handles the "BankDepositGold" message. */
/* ' */
/* ' @param    userIndex The index of the user sending the message. */

void HandleBankDepositGold(int UserIndex) {
	/* '*************************************************** */
	/* 'Author: Juan Martín Sotuyo Dodero (Maraxus) */
	/* 'Last Modification: 05/17/06 */
	/* ' */
	/* '*************************************************** */
	if (UserList[UserIndex].sockctx->incomingData->length() < 5) {
		throw insufficient_data_error();
		return;
	}

	/* 'Remove packet ID */
	UserList[UserIndex].sockctx->incomingData->ReadByte();

	int Amount;

	Amount = UserList[UserIndex].sockctx->incomingData->ReadLong();

	/* 'Dead people can't leave a faction.. they can't talk... */
	if (UserList[UserIndex].flags.Muerto == 1) {
		WriteConsoleMsg(UserIndex, "¡¡Estás muerto!!", FontTypeNames_FONTTYPE_INFO);
		return;
	}

	/* 'Validate target NPC */
	if (UserList[UserIndex].flags.TargetNPC == 0) {
		WriteConsoleMsg(UserIndex,
				"Primero tienes que seleccionar un personaje, haz click izquierdo sobre él.",
				FontTypeNames_FONTTYPE_INFO);
		return;
	}

	if (Distancia(Npclist[UserList[UserIndex].flags.TargetNPC].Pos, UserList[UserIndex].Pos) > 10) {
		WriteConsoleMsg(UserIndex, "Estás demasiado lejos.", FontTypeNames_FONTTYPE_INFO);
		return;
	}

	if (Npclist[UserList[UserIndex].flags.TargetNPC].NPCtype != eNPCType_Banquero) {
		return;
	}

	if (Amount > 0 && Amount <= UserList[UserIndex].Stats.GLD) {
		UserList[UserIndex].Stats.Banco = UserList[UserIndex].Stats.Banco + Amount;
		UserList[UserIndex].Stats.GLD = UserList[UserIndex].Stats.GLD - Amount;
		WriteChatOverHead(UserIndex,
				"Tenés " + vb6::CStr(UserList[UserIndex].Stats.Banco) + " monedas de oro en tu cuenta.",
				Npclist[UserList[UserIndex].flags.TargetNPC].Char.CharIndex, 0x00ffffff);

		WriteUpdateGold(UserIndex);
		WriteUpdateBankGold(UserIndex);
	} else {
		WriteChatOverHead(UserIndex, "No tenés esa cantidad.",
				Npclist[UserList[UserIndex].flags.TargetNPC].Char.CharIndex, 0x00ffffff);
	}
}

/* '' */
/* ' Handles the "Denounce" message. */
/* ' */
/* ' @param    userIndex The index of the user sending the message. */

void HandleDenounce(int UserIndex) {
	/* '*************************************************** */
	/* 'Author: Juan Martín Sotuyo Dodero (Maraxus) */
	/* 'Last Modification: 14/11/2010 */
	/* '14/11/2010: ZaMa - Now denounces can be desactivated. */
	/* '*************************************************** */
	if (UserList[UserIndex].sockctx->incomingData->length() < 3) {
		throw insufficient_data_error();
		return;
	}

	/* 'This packet contains strings, make a copy of the data to prevent losses if it's not complete yet... */
	clsByteQueue* buffer = UserList[UserIndex].sockctx->incomingData.get();


	/* 'Remove packet ID */
	buffer->ReadByte();

	std::string Text;
	std::string msg;

	Text = buffer->ReadUnicodeString();

	if (UserList[UserIndex].flags.Silenciado == 0) {
		/* 'Analize chat... */
		ParseChat(Text);

		msg = vb6::LCase(UserList[UserIndex].Name) + " DENUNCIA: " + Text;

		SendData(SendTarget_ToAdmins, 0, PrepareMessageConsoleMsg(msg, FontTypeNames_FONTTYPE_GUILDMSG),
				true);

		Denuncias.push_back(msg);
		LogDesarrollo("Denuncia de " + UserList[UserIndex].Name + ": " + msg);

		WriteConsoleMsg(UserIndex, "Denuncia enviada, espere..", FontTypeNames_FONTTYPE_INFO);
	}

	/* 'If we got here then packet is complete, copy data back to original queue */


}

/* '' */
/* ' Handles the "GuildFundate" message. */
/* ' */
/* ' @param    userIndex The index of the user sending the message. */

void HandleGuildFundate(int UserIndex) {
	/* '*************************************************** */
	/* 'Author: ZaMa */
	/* 'Last Modification: 14/12/2009 */
	/* ' */
	/* '*************************************************** */
	if (UserList[UserIndex].sockctx->incomingData->length() < 1) {
		throw insufficient_data_error();
		return;
	}

	UserList[UserIndex].sockctx->incomingData->ReadByte();

	if (HasFound(UserList[UserIndex].Name)) {
		WriteConsoleMsg(UserIndex, "¡Ya has fundado un clan, no puedes fundar otro!",
				FontTypeNames_FONTTYPE_INFOBOLD);
		return;
	}

	WriteShowGuildAlign(UserIndex);
}

/* '' */
/* ' Handles the "GuildFundation" message. */
/* ' */
/* ' @param    userIndex The index of the user sending the message. */

void HandleGuildFundation(int UserIndex) {
	/* '*************************************************** */
	/* 'Author: ZaMa */
	/* 'Last Modification: 14/12/2009 */
	/* ' */
	/* '*************************************************** */
	if (UserList[UserIndex].sockctx->incomingData->length() < 2) {
		throw insufficient_data_error();
		return;
	}

	/* 'Remove packet ID */
	UserList[UserIndex].sockctx->incomingData->ReadByte();

	eClanType clanType;
	std::string ERROR;

	clanType = static_cast<eClanType>(UserList[UserIndex].sockctx->incomingData->ReadByte());

	if (HasFound(UserList[UserIndex].Name)) {
		WriteConsoleMsg(UserIndex, "¡Ya has fundado un clan, no puedes fundar otro!",
				FontTypeNames_FONTTYPE_INFOBOLD);
		LogCheating(
				"El usuario " + UserList[UserIndex].Name
						+ " ha intentado fundar un clan ya habiendo fundado otro desde la IP "
						+ UserList[UserIndex].ip);
		return;
	}

	switch (clanType) {
	case eClanType_ct_RoyalArmy:
		UserList[UserIndex].FundandoGuildAlineacion = ALINEACION_GUILD_ALINEACION_ARMADA;
		break;

	case eClanType_ct_Evil:
		UserList[UserIndex].FundandoGuildAlineacion = ALINEACION_GUILD_ALINEACION_LEGION;
		break;

	case eClanType_ct_Neutral:
		UserList[UserIndex].FundandoGuildAlineacion = ALINEACION_GUILD_ALINEACION_NEUTRO;
		break;

	case eClanType_ct_GM:
		UserList[UserIndex].FundandoGuildAlineacion = ALINEACION_GUILD_ALINEACION_MASTER;
		break;

	case eClanType_ct_Legal:
		UserList[UserIndex].FundandoGuildAlineacion = ALINEACION_GUILD_ALINEACION_CIUDA;
		break;

	case eClanType_ct_Criminal:
		UserList[UserIndex].FundandoGuildAlineacion = ALINEACION_GUILD_ALINEACION_CRIMINAL;
		break;

	default:
		WriteConsoleMsg(UserIndex, "Alineación inválida.", FontTypeNames_FONTTYPE_GUILD);
		return;
		break;
	}

	if (PuedeFundarUnClan(UserIndex, UserList[UserIndex].FundandoGuildAlineacion, ERROR)) {
		WriteShowGuildFundationForm(UserIndex);
	} else {
		UserList[UserIndex].FundandoGuildAlineacion = ALINEACION_GUILD_Null;
		WriteConsoleMsg(UserIndex, ERROR, FontTypeNames_FONTTYPE_GUILD);
	}
}

/* '' */
/* ' Handles the "PartyKick" message. */
/* ' */
/* ' @param    userIndex The index of the user sending the message. */

void HandlePartyKick(int UserIndex) {
	/* '*************************************************** */
	/* 'Author: Juan Martín Sotuyo Dodero (Maraxus) */
	/* 'Last Modification: 05/05/09 */
	/* 'Last Modification by: Marco Vanotti (Marco) */
	/* '- 05/05/09: Now it uses "UserPuedeEjecutarComandos" to check if the user can use party commands */
	/* '*************************************************** */
	if (UserList[UserIndex].sockctx->incomingData->length() < 3) {
		throw insufficient_data_error();
		return;
	}

	/* 'This packet contains strings, make a copy of the data to prevent losses if it's not complete yet... */
	clsByteQueue* buffer = UserList[UserIndex].sockctx->incomingData.get();


	/* 'Remove packet ID */
	buffer->ReadByte();

	std::string UserName;
	int tUser = 0;

	UserName = buffer->ReadUnicodeString();

	if (UserPuedeEjecutarComandos(UserIndex)) {
		tUser = NameIndex(UserName);

		if (tUser > 0) {
			ExpulsarDeParty(UserIndex, tUser);
		} else {
			if (vb6::InStr(UserName, "+")) {
				UserName = vb6::Replace(UserName, "+", " ");
			}

			WriteConsoleMsg(UserIndex, vb6::LCase(UserName) + " no pertenece a tu party.",
					FontTypeNames_FONTTYPE_INFO);
		}
	}

	/* 'If we got here then packet is complete, copy data back to original queue */


}

/* '' */
/* ' Handles the "PartySetLeader" message. */
/* ' */
/* ' @param    userIndex The index of the user sending the message. */

void HandlePartySetLeader(int UserIndex) {
	/* '*************************************************** */
	/* 'Author: Juan Martín Sotuyo Dodero (Maraxus) */
	/* 'Last Modification: 05/05/09 */
	/* 'Last Modification by: Marco Vanotti (MarKoxX) */
	/* '- 05/05/09: Now it uses "UserPuedeEjecutarComandos" to check if the user can use party commands */
	/* '*************************************************** */
	if (UserList[UserIndex].sockctx->incomingData->length() < 3) {
		throw insufficient_data_error();
		return;
	}

	/* 'This packet contains strings, make a copy of the data to prevent losses if it's not complete yet... */
	clsByteQueue* buffer = UserList[UserIndex].sockctx->incomingData.get();


	/* 'Remove packet ID */
	buffer->ReadByte();

	std::string UserName;
	int tUser = 0;
	//int rank;
	//rank = PlayerType_Admin || PlayerType_Dios || PlayerType_SemiDios || PlayerType_Consejero;

	UserName = buffer->ReadUnicodeString();
	if (UserPuedeEjecutarComandos(UserIndex)) {
		tUser = NameIndex(UserName);
		if (tUser > 0) {
			/* 'Don't allow users to spoof online GMs */
			if (!UserTieneMasPrivilegiosQue(UserName, UserIndex)) {
				TransformarEnLider(UserIndex, tUser);
			} else {
				WriteConsoleMsg(UserIndex, vb6::LCase(UserList[tUser].Name) + " no pertenece a tu party.",
						FontTypeNames_FONTTYPE_INFO);
			}

		} else {
			if (vb6::InStr(UserName, "+")) {
				UserName = vb6::Replace(UserName, "+", " ");
			}
			WriteConsoleMsg(UserIndex, vb6::LCase(UserName) + " no pertenece a tu party.",
					FontTypeNames_FONTTYPE_INFO);
		}
	}

	/* 'If we got here then packet is complete, copy data back to original queue */


}

/* '' */
/* ' Handles the "PartyAcceptMember" message. */
/* ' */
/* ' @param    userIndex The index of the user sending the message. */

void HandlePartyAcceptMember(int UserIndex) {
	/* '*************************************************** */
	/* 'Author: Juan Martín Sotuyo Dodero (Maraxus) */
	/* 'Last Modification: 05/05/09 */
	/* 'Last Modification by: Marco Vanotti (Marco) */
	/* '- 05/05/09: Now it uses "UserPuedeEjecutarComandos" to check if the user can use party commands */
	/* '*************************************************** */
	if (UserList[UserIndex].sockctx->incomingData->length() < 3) {
		throw insufficient_data_error();
		return;
	}

	/* 'This packet contains strings, make a copy of the data to prevent losses if it's not complete yet... */
	clsByteQueue* buffer = UserList[UserIndex].sockctx->incomingData.get();


	/* 'Remove packet ID */
	buffer->ReadByte();

	std::string UserName;
	int tUser = 0;
	bool bUserVivo = false;

	UserName = buffer->ReadUnicodeString();
	if (UserList[UserIndex].flags.Muerto) {
		WriteConsoleMsg(UserIndex, "¡¡Estás muerto!!", FontTypeNames_FONTTYPE_PARTY);
	} else {
		bUserVivo = true;
	}

	if (UserPuedeEjecutarComandos(UserIndex) && bUserVivo) {
		tUser = NameIndex(UserName);
		if (tUser > 0) {
			/* 'Validate administrative ranks - don't allow users to spoof online GMs */
			if (!UserTieneMasPrivilegiosQue(tUser, UserIndex)) {
				AprobarIngresoAParty(UserIndex, tUser);
			} else {
				WriteConsoleMsg(UserIndex, "No puedes incorporar a tu party a personajes de mayor jerarquía.",
						FontTypeNames_FONTTYPE_INFO);
			}
		} else {
			if (vb6::InStr(UserName, "+")) {
				UserName = vb6::Replace(UserName, "+", " ");
			}

			/* 'Don't allow users to spoof online GMs */
			if (!UserTieneMasPrivilegiosQue(UserName, UserIndex)) {
				WriteConsoleMsg(UserIndex, vb6::LCase(UserName) + " no ha solicitado ingresar a tu party.",
						FontTypeNames_FONTTYPE_PARTY);
			} else {
				WriteConsoleMsg(UserIndex, "No puedes incorporar a tu party a personajes de mayor jerarquía.",
						FontTypeNames_FONTTYPE_INFO);
			}
		}
	}

	/* 'If we got here then packet is complete, copy data back to original queue */


}

/* '' */
/* ' Handles the "GuildMemberList" message. */
/* ' */
/* ' @param    userIndex The index of the user sending the message. */

void HandleGuildMemberList(int UserIndex) {
	/* '*************************************************** */
	/* 'Author: Juan Martín Sotuyo Dodero (Maraxus) */
	/* 'Last Modification: 05/17/06 */
	/* ' */
	/* '*************************************************** */
	if (UserList[UserIndex].sockctx->incomingData->length() < 3) {
		throw insufficient_data_error();
		return;
	}

	/* 'This packet contains strings, make a copy of the data to prevent losses if it's not complete yet... */
	clsByteQueue* buffer = UserList[UserIndex].sockctx->incomingData.get();


	/* 'Remove packet ID */
	buffer->ReadByte();

	std::string guild;
	int memberCount;
	int i;
	std::string UserName;

	guild = buffer->ReadUnicodeString();

	if (UserTieneAlgunPrivilegios(UserIndex, PlayerType_Admin, PlayerType_Dios)) {
		if ((vb6::InStrB(guild, "/") != 0)) {
			guild = vb6::Replace(guild, "/", "");
		}
		if ((vb6::InStrB(guild, "/") != 0)) {
			guild = vb6::Replace(guild, "/", "");
		}

		if (!FileExist(GetGuildsPath(guild, EGUILDPATH::Members))) {
			WriteConsoleMsg(UserIndex, "No existe el clan: " + guild, FontTypeNames_FONTTYPE_INFO);
		} else {
			memberCount = vb6::val(
					GetVar(GetGuildsPath(guild, EGUILDPATH::Members), "INIT",
							"NroMembers"));

			for (i = (1); i <= (memberCount); i++) {
				UserName = GetVar(GetGuildsPath(guild, EGUILDPATH::Members), "Members",
						"Member" + vb6::CStr(i));

				WriteConsoleMsg(UserIndex, UserName + "<" + guild + ">", FontTypeNames_FONTTYPE_INFO);
			}
		}
	}

	/* 'If we got here then packet is complete, copy data back to original queue */


}

/* '' */
/* ' Handles the "GMMessage" message. */
/* ' */
/* ' @param    userIndex The index of the user sending the message. */

void HandleGMMessage(int UserIndex) {
	/* '*************************************************** */
	/* 'Author: Juan Martín Sotuyo Dodero (Maraxus) */
	/* 'Last Modification: 01/08/07 */
	/* 'Last Modification by: (liquid) */
	/* '*************************************************** */
	if (UserList[UserIndex].sockctx->incomingData->length() < 3) {
		throw insufficient_data_error();
		return;
	}

	/* 'This packet contains strings, make a copy of the data to prevent losses if it's not complete yet... */
	clsByteQueue* buffer = UserList[UserIndex].sockctx->incomingData.get();


	/* 'Remove packet ID */
	buffer->ReadByte();

	std::string message;

	message = buffer->ReadUnicodeString();

	if (!UserTieneAlgunPrivilegios(UserIndex, PlayerType_User)) {
		LogGM(UserList[UserIndex].Name, "Mensaje a Gms:" + message);

		if (vb6::LenB(message) != 0) {
			/* 'Analize chat... */
			ParseChat(message);

			SendData(SendTarget_ToAdmins, 0,
					PrepareMessageConsoleMsg(UserList[UserIndex].Name + "> " + message,
							FontTypeNames_FONTTYPE_GMMSG));
		}
	}

	/* 'If we got here then packet is complete, copy data back to original queue */


}

/* '' */
/* ' Handles the "ShowName" message. */
/* ' */
/* ' @param    userIndex The index of the user sending the message. */

void HandleShowName(int UserIndex) {
	/* '*************************************************** */
	/* 'Author: Juan Martín Sotuyo Dodero (Maraxus) */
	/* 'Last Modification: 05/17/06 */
	/* ' */
	/* '*************************************************** */
	/* 'Remove packet ID */
	UserList[UserIndex].sockctx->incomingData->ReadByte();

	if (UserTieneAlgunPrivilegios(UserIndex, PlayerType_Dios, PlayerType_Admin, PlayerType_RoleMaster)) {
		/* 'Show / Hide the name */
		UserList[UserIndex].showName = !UserList[UserIndex].showName;

		RefreshCharStatus(UserIndex);
	}
}

/* '' */
/* ' Handles the "OnlineRoyalArmy" message. */
/* ' */
/* ' @param    userIndex The index of the user sending the message. */

void HandleOnlineRoyalArmy(int UserIndex) {
	/* '*************************************************** */
	/* 'Author: Juan Martín Sotuyo Dodero (Maraxus) */
	/* 'Last Modification: 28/05/2010 */
	/* '28/05/2010: ZaMa - Ahora solo dioses pueden ver otros dioses online. */
	/* '*************************************************** */
	/* 'Remove packet ID */
	UserList[UserIndex].sockctx->incomingData->ReadByte();

	if (UserTieneAlgunPrivilegios(UserIndex, PlayerType_User)) {
		return;
	}

	int i;
	std::string list;
	bool esgm = false;

	/* ' Solo dioses pueden ver otros dioses online */
	if (UserTieneAlgunPrivilegios(UserIndex, PlayerType_Dios, PlayerType_Admin)) {
		esgm = true;
	}

	for (i = (1); i <= (LastUser); i++) {
		if (UserList[i].ConnID != -1) {
			if (UserList[i].Faccion.ArmadaReal == 1) {
				if (esgm || UserTieneAlgunPrivilegios(UserIndex, PlayerType_User, PlayerType_Consejero, PlayerType_SemiDios)) {
					list = list + UserList[i].Name + ", ";
				}
			}
		}
	}

	if (vb6::Len(list) > 0) {
		WriteConsoleMsg(UserIndex, "Reales conectados: " + vb6::Left(list, vb6::Len(list) - 2),
				FontTypeNames_FONTTYPE_INFO);
	} else {
		WriteConsoleMsg(UserIndex, "No hay reales conectados.", FontTypeNames_FONTTYPE_INFO);
	}
}

/* '' */
/* ' Handles the "OnlineChaosLegion" message. */
/* ' */
/* ' @param    userIndex The index of the user sending the message. */

void HandleOnlineChaosLegion(int UserIndex) {
	/* '*************************************************** */
	/* 'Author: Juan Martín Sotuyo Dodero (Maraxus) */
	/* 'Last Modification: 28/05/2010 */
	/* '28/05/2010: ZaMa - Ahora solo dioses pueden ver otros dioses online. */
	/* '*************************************************** */
	/* 'Remove packet ID */
	UserList[UserIndex].sockctx->incomingData->ReadByte();

	if (UserTieneAlgunPrivilegios(UserIndex, PlayerType_User)) {
		return;
	}

	int i;
	std::string list;
	bool esgm = false;

	/* ' Solo dioses pueden ver otros dioses online */
	if (UserTieneAlgunPrivilegios(UserIndex, PlayerType_Dios, PlayerType_Admin)) {
		esgm = true;
	}

	for (i = (1); i <= (LastUser); i++) {
		if (UserList[i].ConnID != -1) {
			if (UserList[i].Faccion.FuerzasCaos == 1) {
				if (UserTieneAlgunPrivilegios(i, PlayerType_User, PlayerType_Consejero, PlayerType_SemiDios) || esgm) {
					list = list + UserList[i].Name + ", ";
				}
			}
		}
	}

	if (vb6::Len(list) > 0) {
		WriteConsoleMsg(UserIndex, "Caos conectados: " + vb6::Left(list, vb6::Len(list) - 2),
				FontTypeNames_FONTTYPE_INFO);
	} else {
		WriteConsoleMsg(UserIndex, "No hay Caos conectados.", FontTypeNames_FONTTYPE_INFO);
	}
}

/* '' */
/* ' Handles the "GoNearby" message. */
/* ' */
/* ' @param    userIndex The index of the user sending the message. */

void HandleGoNearby(int UserIndex) {
	/* '*************************************************** */
	/* 'Author: Juan Martín Sotuyo Dodero (Maraxus) */
	/* 'Last Modification: 01/10/07 */
	/* ' */
	/* '*************************************************** */
	if (UserList[UserIndex].sockctx->incomingData->length() < 3) {
		throw insufficient_data_error();
		return;
	}

	/* 'This packet contains strings, make a copy of the data to prevent losses if it's not complete yet... */
	clsByteQueue* buffer = UserList[UserIndex].sockctx->incomingData.get();


	/* 'Remove packet ID */
	buffer->ReadByte();

	std::string UserName;

	UserName = buffer->ReadUnicodeString();

	int tIndex;
	int X;
	int Y;
	int i;
	bool Found = false;

	tIndex = NameIndex(UserName);

	/* 'Check the user has enough powers */
	if (UserTieneAlgunPrivilegios(UserIndex, PlayerType_Admin, PlayerType_Dios, PlayerType_SemiDios, PlayerType_Consejero)) {
		/* 'Si es dios o Admins no podemos salvo que nosotros también lo seamos */
		if (!(EsDios(UserName) || EsAdmin(UserName))
				|| (UserTieneAlgunPrivilegios(UserIndex, PlayerType_Dios, PlayerType_Admin))) {
			/* 'existe el usuario destino? */
			if (tIndex <= 0) {
				WriteConsoleMsg(UserIndex, "Usuario offline.", FontTypeNames_FONTTYPE_INFO);
			} else {
				/* 'esto for sirve ir cambiando la distancia destino */
				for (i = (2); i <= (5); i++) {
					for (X = (UserList[tIndex].Pos.X - i); X <= (UserList[tIndex].Pos.X + i); X++) {
						for (Y = (UserList[tIndex].Pos.Y - i); Y <= (UserList[tIndex].Pos.Y + i); Y++) {
							if (MapData[UserList[tIndex].Pos.Map][X][Y].UserIndex == 0) {
								if (LegalPos(UserList[tIndex].Pos.Map, X, Y, true, true)) {
									WarpUserChar(UserIndex, UserList[tIndex].Pos.Map, X, Y, true);
									LogGM(UserList[UserIndex].Name,
											"/IRCERCA " + UserName + " Mapa:" + vb6::CStr(UserList[tIndex].Pos.Map)
													+ " X:" + vb6::CStr(UserList[tIndex].Pos.X) + " Y:"
													+ vb6::CStr(UserList[tIndex].Pos.Y));
									Found = true;
									break;
								}
							}
						}

						/* ' Feo, pero hay que abortar 3 fors sin usar GoTo */
						if (Found) {
							break;
						}
					}

					/* ' Feo, pero hay que abortar 3 fors sin usar GoTo */
					if (Found) {
						break;
					}
				}

				/* 'No space found?? */
				if (!Found) {
					WriteConsoleMsg(UserIndex, "Todos los lugares están ocupados.",
							FontTypeNames_FONTTYPE_INFO);
				}
			}
		}
	}

	/* 'If we got here then packet is complete, copy data back to original queue */


}

/* '' */
/* ' Handles the "Comment" message. */
/* ' */
/* ' @param    userIndex The index of the user sending the message. */

void HandleComment(int UserIndex) {
	/* '*************************************************** */
	/* 'Author: Juan Martín Sotuyo Dodero (Maraxus) */
	/* 'Last Modification: 05/17/06 */
	/* ' */
	/* '*************************************************** */
	if (UserList[UserIndex].sockctx->incomingData->length() < 3) {
		throw insufficient_data_error();
		return;
	}

	/* 'This packet contains strings, make a copy of the data to prevent losses if it's not complete yet... */
	clsByteQueue* buffer = UserList[UserIndex].sockctx->incomingData.get();


	/* 'Remove packet ID */
	buffer->ReadByte();

	std::string comment;
	comment = buffer->ReadUnicodeString();

	if (!UserTieneAlgunPrivilegios(UserIndex, PlayerType_User)) {
		LogGM(UserList[UserIndex].Name, "Comentario: " + comment);
		WriteConsoleMsg(UserIndex, "Comentario salvado...", FontTypeNames_FONTTYPE_INFO);
	}

	/* 'If we got here then packet is complete, copy data back to original queue */


}

/* '' */
/* ' Handles the "ServerTime" message. */
/* ' */
/* ' @param    userIndex The index of the user sending the message. */

void HandleServerTime(int UserIndex) {
	/* '*************************************************** */
	/* 'Author: Juan Martín Sotuyo Dodero (Maraxus) */
	/* 'Last Modification: 01/08/07 */
	/* 'Last Modification by: (liquid) */
	/* '*************************************************** */
	/* 'Remove packet ID */
	UserList[UserIndex].sockctx->incomingData->ReadByte();

	if (UserTieneAlgunPrivilegios(UserIndex, PlayerType_User)) {
		return;
	}

	LogGM(UserList[UserIndex].Name, "Hora.");

	SendData(SendTarget_ToAll, 0,
			PrepareMessageConsoleMsg("Hora: " + vb6::CStr(vb6::Now()), FontTypeNames_FONTTYPE_INFO));
}

/* '' */
/* ' Handles the "Where" message. */
/* ' */
/* ' @param    userIndex The index of the user sending the message. */

void HandleWhere(int UserIndex) {
	/* '*************************************************** */
	/* 'Author: Juan Martín Sotuyo Dodero (Maraxus) */
	/* 'Last Modification: 18/11/2010 */
	/* '07/06/2010: ZaMa - Ahora no se puede usar para saber si hay dioses/admins online. */
	/* '18/11/2010: ZaMa - Obtengo los privs del charfile antes de mostrar la posicion de un usuario offline. */
	/* '*************************************************** */
	if (UserList[UserIndex].sockctx->incomingData->length() < 3) {
		throw insufficient_data_error();
		return;
	}

	/* 'This packet contains strings, make a copy of the data to prevent losses if it's not complete yet... */
	clsByteQueue* buffer = UserList[UserIndex].sockctx->incomingData.get();


	/* 'Remove packet ID */
	buffer->ReadByte();

	std::string UserName;
	int tUser = 0;
	std::string miPos;

	UserName = buffer->ReadUnicodeString();

	if (!UserTieneAlgunPrivilegios(UserIndex, PlayerType_User)) {

		tUser = NameIndex(UserName);
		if (tUser <= 0) {

			if (FileExist(GetCharPath(UserName), 0)) {

				PlayerType CharPrivs;
				CharPrivs = GetCharPrivs(UserName);

				if ((CharPrivs & (PlayerType_User | PlayerType_Consejero | PlayerType_SemiDios)) != 0
						|| (((CharPrivs & (PlayerType_Dios | PlayerType_Admin)) != 0)
								&& (UserTieneAlgunPrivilegios(UserIndex, PlayerType_Dios, PlayerType_Admin)))) {
					miPos = GetVar(GetCharPath(UserName), "INIT", "POSITION");
					WriteConsoleMsg(UserIndex,
							"Ubicación  " + UserName + " (Offline): " + ReadField(1, miPos, 45) + ", "
									+ ReadField(2, miPos, 45) + ", " + ReadField(3, miPos, 45) + ".",
							FontTypeNames_FONTTYPE_INFO);
				}
			} else {
				if (!(EsDios(UserName) || EsAdmin(UserName))) {
					WriteConsoleMsg(UserIndex, "Usuario inexistente.", FontTypeNames_FONTTYPE_INFO);
				} else if (UserTieneAlgunPrivilegios(UserIndex, PlayerType_Dios, PlayerType_Admin)) {
					WriteConsoleMsg(UserIndex, "Usuario inexistente.", FontTypeNames_FONTTYPE_INFO);
				}
			}
		} else {
			if ((UserTieneAlgunPrivilegios(tUser, PlayerType_User, PlayerType_Consejero, PlayerType_SemiDios))
					|| (((UserTieneAlgunPrivilegios(tUser, PlayerType_Dios, PlayerType_Admin)))
							&& (UserTieneAlgunPrivilegios(UserIndex, PlayerType_Dios, PlayerType_Admin)))) {
				WriteConsoleMsg(UserIndex,
						"Ubicación  " + UserName + ": " + vb6::CStr(UserList[tUser].Pos.Map) + ", "
								+ vb6::CStr(UserList[tUser].Pos.X) + ", " + vb6::CStr(UserList[tUser].Pos.Y) + ".",
						FontTypeNames_FONTTYPE_INFO);
			}
		}

		LogGM(UserList[UserIndex].Name, "/Donde " + UserName);
	}

	/* 'If we got here then packet is complete, copy data back to original queue */


}

/* '' */
/* ' Handles the "CreaturesInMap" message. */
/* ' */
/* ' @param    userIndex The index of the user sending the message. */

void HandleCreaturesInMap(int UserIndex) {
	/* '*************************************************** */
	/* 'Author: Juan Martín Sotuyo Dodero (Maraxus) */
	/* 'Last Modification: 30/07/06 */
	/* 'Pablo (ToxicWaste): modificaciones generales para simplificar la visualización. */
	/* '*************************************************** */
	if (UserList[UserIndex].sockctx->incomingData->length() < 3) {
		throw insufficient_data_error();
		return;
	}

	/* 'Remove packet ID */
	UserList[UserIndex].sockctx->incomingData->ReadByte();

	int Map;
	int i;
	int j;
	int NPCcount1 = 0;
	int NPCcount2 = 0;
	vb6::array<int> NPCcant1;
	vb6::array<int> NPCcant2;
	vb6::array<std::string> List1;
	vb6::array<std::string> List2;

	Map = UserList[UserIndex].sockctx->incomingData->ReadInteger();

	if (UserTieneAlgunPrivilegios(UserIndex, PlayerType_User)) {
		return;
	}

	if (MapaValido(Map)) {
		for (i = (1); i <= (LastNPC); i++) {
			/* 'VB isn't lazzy, so we put more restrictive condition first to speed up the process */
			if (Npclist[i].Pos.Map == Map) {
				/* '¿esta vivo? */
				if (Npclist[i].flags.NPCActive && Npclist[i].Hostile == 1
						&& Npclist[i].Stats.Alineacion == 2) {
					if (NPCcount1 == 0) {
						List1.redim(0);
						List1.redim(0);
						NPCcant1.redim(0);
						NPCcant1.redim(0);
						NPCcount1 = 1;
						List1[0] = Npclist[i].Name + ": (" + vb6::CStr(Npclist[i].Pos.X) + "," + vb6::CStr(Npclist[i].Pos.Y) + ")";
						NPCcant1[0] = 1;
					} else {
						for (j = (0); j <= (NPCcount1 - 1); j++) {
							if (vb6::Left(List1[j], vb6::Len(Npclist[i].Name)) == Npclist[i].Name) {
								List1[j] = List1[j] + ", (" + vb6::CStr(Npclist[i].Pos.X) + "," + vb6::CStr(Npclist[i].Pos.Y) + ")";
								NPCcant1[j] = NPCcant1[j] + 1;
								break;
							}
						}
						if (j == NPCcount1) {
							List1.redim(NPCcount1);
							NPCcant1.redim(NPCcount1);
							NPCcount1 = NPCcount1 + 1;
							List1[j] = Npclist[i].Name + ": (" + vb6::CStr(Npclist[i].Pos.X) + "," + vb6::CStr(Npclist[i].Pos.Y)
									+ ")";
							NPCcant1[j] = 1;
						}
					}
				} else {
					if (NPCcount2 == 0) {
						List2.redim(0);
						List2.redim(0);
						NPCcant2.redim(0);
						NPCcant2.redim(0);
						NPCcount2 = 1;
						List2[0] = Npclist[i].Name + ": (" + vb6::CStr(Npclist[i].Pos.X) + "," + vb6::CStr(Npclist[i].Pos.Y) + ")";
						NPCcant2[0] = 1;
					} else {
						for (j = (0); j <= (NPCcount2 - 1); j++) {
							if (vb6::Left(List2[j], vb6::Len(Npclist[i].Name)) == Npclist[i].Name) {
								List2[j] = List2[j] + ", (" + vb6::CStr(Npclist[i].Pos.X) + "," + vb6::CStr(Npclist[i].Pos.Y) + ")";
								NPCcant2[j] = NPCcant2[j] + 1;
								break;
							}
						}
						if (j == NPCcount2) {
							List2.redim(NPCcount2);
							NPCcant2.redim(NPCcount2);
							NPCcount2 = NPCcount2 + 1;
							List2[j] = Npclist[i].Name + ": (" + vb6::CStr(Npclist[i].Pos.X) + "," + vb6::CStr(Npclist[i].Pos.Y)
									+ ")";
							NPCcant2[j] = 1;
						}
					}
				}
			}
		}

		WriteConsoleMsg(UserIndex, "Npcs Hostiles en mapa: ", FontTypeNames_FONTTYPE_WARNING);
		if (NPCcount1 == 0) {
			WriteConsoleMsg(UserIndex, "No hay NPCS Hostiles.", FontTypeNames_FONTTYPE_INFO);
		} else {
			for (j = (0); j <= (NPCcount1 - 1); j++) {
				WriteConsoleMsg(UserIndex, vb6::CStr(NPCcant1[j]) + " " + vb6::CStr(List1[j]), FontTypeNames_FONTTYPE_INFO);
			}
		}
		WriteConsoleMsg(UserIndex, "Otros Npcs en mapa: ", FontTypeNames_FONTTYPE_WARNING);
		if (NPCcount2 == 0) {
			WriteConsoleMsg(UserIndex, "No hay más NPCS.", FontTypeNames_FONTTYPE_INFO);
		} else {
			for (j = (0); j <= (NPCcount2 - 1); j++) {
				WriteConsoleMsg(UserIndex, vb6::CStr(NPCcant2[j]) + " " + vb6::CStr(List2[j]), FontTypeNames_FONTTYPE_INFO);
			}
		}
		LogGM(UserList[UserIndex].Name, "Numero enemigos en mapa " + vb6::CStr(Map));
	}
}

/* '' */
/* ' Handles the "WarpMeToTarget" message. */
/* ' */
/* ' @param    userIndex The index of the user sending the message. */

void HandleWarpMeToTarget(int UserIndex) {
	/* '*************************************************** */
	/* 'Author: Juan Martín Sotuyo Dodero (Maraxus) */
	/* 'Last Modification: 26/03/09 */
	/* '26/03/06: ZaMa - Chequeo que no se teletransporte donde haya un char o npc */
	/* '*************************************************** */
	/* 'Remove packet ID */
	UserList[UserIndex].sockctx->incomingData->ReadByte();

	int X;
	int Y;

	if (UserTieneAlgunPrivilegios(UserIndex, PlayerType_User)) {
		return;
	}

	X = UserList[UserIndex].flags.TargetX;
	Y = UserList[UserIndex].flags.TargetY;

	FindLegalPos(UserIndex, UserList[UserIndex].flags.TargetMap, X, Y);
	WarpUserChar(UserIndex, UserList[UserIndex].flags.TargetMap, X, Y, true);
	LogGM(UserList[UserIndex].Name,
			"/TELEPLOC a x:" + vb6::CStr(UserList[UserIndex].flags.TargetX) + " Y:" + vb6::CStr(UserList[UserIndex].flags.TargetY)
					+ " Map:" + vb6::CStr(UserList[UserIndex].Pos.Map));
}

/* '' */
/* ' Handles the "WarpChar" message. */
/* ' */
/* ' @param    userIndex The index of the user sending the message. */

void HandleWarpChar(int UserIndex) {
	/* '*************************************************** */
	/* 'Author: Juan Martín Sotuyo Dodero (Maraxus) */
	/* 'Last Modification: 26/03/2009 */
	/* '26/03/2009: ZaMa -  Chequeo que no se teletransporte a un tile donde haya un char o npc. */
	/* '*************************************************** */
	if (UserList[UserIndex].sockctx->incomingData->length() < 7) {
		throw insufficient_data_error();
		return;
	}

	/* 'This packet contains strings, make a copy of the data to prevent losses if it's not complete yet... */
	clsByteQueue* buffer = UserList[UserIndex].sockctx->incomingData.get();


	/* 'Remove packet ID */
	buffer->ReadByte();

	std::string UserName;
	int Map;
	int X;
	int Y;
	int tUser = 0;

	UserName = buffer->ReadUnicodeString();
	Map = buffer->ReadInteger();
	X = buffer->ReadByte();
	Y = buffer->ReadByte();

	UserName = vb6::Replace(UserName, "+", " ");

	if (!UserTieneAlgunPrivilegios(UserIndex, PlayerType_User)) {
		if (MapaValido(Map) && vb6::LenB(UserName) != 0) {
			if (vb6::UCase(UserName) != "YO") {
				if (!UserTieneAlgunPrivilegios(UserIndex, PlayerType_Consejero)) {
					tUser = NameIndex(UserName);
				}
			} else {
				tUser = UserIndex;
			}

			if (tUser <= 0) {
				if (!(EsDios(UserName) || EsAdmin(UserName))) {
					WriteConsoleMsg(UserIndex, "Usuario offline.", FontTypeNames_FONTTYPE_INFO);
				} else {
					WriteConsoleMsg(UserIndex, "No puedes transportar dioses o admins.",
							FontTypeNames_FONTTYPE_INFO);
				}

			} else if (!((UserTieneAlgunPrivilegios(tUser, PlayerType_Dios)) || (UserTieneAlgunPrivilegios(tUser, PlayerType_Admin))) || tUser == UserIndex) {

				if (InMapBounds(Map, X, Y)) {
					FindLegalPos(tUser, Map, X, Y);
					WarpUserChar(tUser, Map, X, Y, true, true);
					WriteConsoleMsg(UserIndex, UserList[tUser].Name + " transportado.",
							FontTypeNames_FONTTYPE_INFO);
					LogGM(UserList[UserIndex].Name,
							"Transportó a " + UserList[tUser].Name + " hacia " + "Mapa" + vb6::CStr(Map) + " X:" + vb6::CStr(X)
									+ " Y:" + vb6::CStr(Y));
				}
			} else {
				WriteConsoleMsg(UserIndex, "No puedes transportar dioses o admins.",
						FontTypeNames_FONTTYPE_INFO);
			}
		}
	}

	/* 'If we got here then packet is complete, copy data back to original queue */


}

/* '' */
/* ' Handles the "Silence" message. */
/* ' */
/* ' @param    userIndex The index of the user sending the message. */

void HandleSilence(int UserIndex) {
	/* '*************************************************** */
	/* 'Author: Juan Martín Sotuyo Dodero (Maraxus) */
	/* 'Last Modification: 05/17/06 */
	/* ' */
	/* '*************************************************** */
	if (UserList[UserIndex].sockctx->incomingData->length() < 3) {
		throw insufficient_data_error();
		return;
	}

	/* 'This packet contains strings, make a copy of the data to prevent losses if it's not complete yet... */
	clsByteQueue* buffer = UserList[UserIndex].sockctx->incomingData.get();


	/* 'Remove packet ID */
	buffer->ReadByte();

	std::string UserName;
	int tUser = 0;

	UserName = buffer->ReadUnicodeString();

	if (!UserTieneAlgunPrivilegios(UserIndex, PlayerType_User)) {
		tUser = NameIndex(UserName);

		if (tUser <= 0) {
			WriteConsoleMsg(UserIndex, "Usuario offline.", FontTypeNames_FONTTYPE_INFO);
		} else {
			if (UserList[tUser].flags.Silenciado == 0) {
				UserList[tUser].flags.Silenciado = 1;
				WriteConsoleMsg(UserIndex, "Usuario silenciado.", FontTypeNames_FONTTYPE_INFO);
				WriteShowMessageBox(tUser,
						"Estimado usuario, ud. ha sido silenciado por los administradores. Sus denuncias serán ignoradas por el servidor de aquí en más. Utilice /GM para contactar un administrador.");
				LogGM(UserList[UserIndex].Name, "/silenciar " + UserList[tUser].Name);

				/* 'Flush the other user's buffer */
				FlushBuffer(tUser);
			} else {
				UserList[tUser].flags.Silenciado = 0;
				WriteConsoleMsg(UserIndex, "Usuario des silenciado.", FontTypeNames_FONTTYPE_INFO);
				LogGM(UserList[UserIndex].Name, "/DESsilenciar " + UserList[tUser].Name);
			}
		}
	}

	/* 'If we got here then packet is complete, copy data back to original queue */


}

/* '' */
/* ' Handles the "SOSShowList" message. */
/* ' */
/* ' @param    userIndex The index of the user sending the message. */

void HandleSOSShowList(int UserIndex) {
	/* '*************************************************** */
	/* 'Author: Juan Martín Sotuyo Dodero (Maraxus) */
	/* 'Last Modification: 05/17/06 */
	/* ' */
	/* '*************************************************** */
	/* 'Remove packet ID */
	UserList[UserIndex].sockctx->incomingData->ReadByte();

	if (UserTieneAlgunPrivilegios(UserIndex, PlayerType_User)) {
		return;
	}
	WriteShowSOSForm(UserIndex);
}

/* '' */
/* ' Handles the "RequestPartyForm" message. */
/* ' */
/* ' @param    userIndex The index of the user sending the message. */

void HandlePartyForm(int UserIndex) {
	/* '*************************************************** */
	/* 'Author: Budi */
	/* 'Last Modification: 11/26/09 */
	/* ' */
	/* '*************************************************** */
	/* 'Remove packet ID */
	UserList[UserIndex].sockctx->incomingData->ReadByte();
	if (UserList[UserIndex].PartyIndex > 0) {
		WriteShowPartyForm(UserIndex);

	} else {
		WriteConsoleMsg(UserIndex, "No perteneces a ningún grupo!", FontTypeNames_FONTTYPE_INFOBOLD);
	}
}

/* '' */
/* ' Handles the "ItemUpgrade" message. */
/* ' */
/* ' @param    UserIndex The index of the user sending the message. */

void HandleItemUpgrade(int UserIndex) {
	/* '*************************************************** */
	/* 'Author: Torres Patricio */
	/* 'Last Modification: 12/09/09 */
	/* ' */
	/* '*************************************************** */
	int ItemIndex;

	/* 'Remove packet ID */
	UserList[UserIndex].sockctx->incomingData->ReadByte();

	ItemIndex = UserList[UserIndex].sockctx->incomingData->ReadInteger();

	if (ItemIndex <= 0) {
		return;
	}
	if (!TieneObjetos(ItemIndex, 1, UserIndex)) {
		return;
	}

	if (!IntervaloPermiteTrabajar(UserIndex)) {
		return;
	}
	DoUpgrade(UserIndex, ItemIndex);
}

/* '' */
/* ' Handles the "SOSRemove" message. */
/* ' */
/* ' @param    userIndex The index of the user sending the message. */

void HandleSOSRemove(int UserIndex) {
	/* '*************************************************** */
	/* 'Author: Juan Martín Sotuyo Dodero (Maraxus) */
	/* 'Last Modification: 05/17/06 */
	/* ' */
	/* '*************************************************** */
	if (UserList[UserIndex].sockctx->incomingData->length() < 3) {
		throw insufficient_data_error();
		return;
	}

	/* 'This packet contains strings, make a copy of the data to prevent losses if it's not complete yet... */
	clsByteQueue* buffer = UserList[UserIndex].sockctx->incomingData.get();


	/* 'Remove packet ID */
	buffer->ReadByte();

	std::string UserName;
	UserName = buffer->ReadUnicodeString();

	if (!UserTieneAlgunPrivilegios(UserIndex, PlayerType_User)) {
		auto it = std::find(Ayuda.begin(), Ayuda.end(), UserName);
		if (it != Ayuda.end()) {
			Ayuda.erase(it);
		}
	}

	/* 'If we got here then packet is complete, copy data back to original queue */


}

/* '' */
/* ' Handles the "GoToChar" message. */
/* ' */
/* ' @param    userIndex The index of the user sending the message. */

void HandleGoToChar(int UserIndex) {
	/* '*************************************************** */
	/* 'Author: Juan Martín Sotuyo Dodero (Maraxus) */
	/* 'Last Modification: 26/03/2009 */
	/* '26/03/2009: ZaMa -  Chequeo que no se teletransporte a un tile donde haya un char o npc. */
	/* '*************************************************** */
	if (UserList[UserIndex].sockctx->incomingData->length() < 3) {
		throw insufficient_data_error();
		return;
	}

	/* 'This packet contains strings, make a copy of the data to prevent losses if it's not complete yet... */
	clsByteQueue* buffer = UserList[UserIndex].sockctx->incomingData.get();


	/* 'Remove packet ID */
	buffer->ReadByte();

	std::string UserName;
	int tUser = 0;
	int X;
	int Y;

	UserName = buffer->ReadUnicodeString();
	tUser = NameIndex(UserName);

	if (UserTieneAlgunPrivilegios(UserIndex, PlayerType_Dios, PlayerType_Admin, PlayerType_SemiDios, PlayerType_Consejero)) {
		/* 'Si es dios o Admins no podemos salvo que nosotros también lo seamos */
		if ((!(EsDios(UserName) || EsAdmin(UserName)))
				|| (UserTieneAlgunPrivilegios(UserIndex, PlayerType_Dios, PlayerType_Admin)
						&& !UserTieneAlgunPrivilegios(UserIndex, PlayerType_RoleMaster))) {
			if (tUser <= 0) {
				WriteConsoleMsg(UserIndex, "Usuario offline.", FontTypeNames_FONTTYPE_INFO);
			} else {
				X = UserList[tUser].Pos.X;
				Y = UserList[tUser].Pos.Y + 1;
				FindLegalPos(UserIndex, UserList[tUser].Pos.Map, X, Y);

				WarpUserChar(UserIndex, UserList[tUser].Pos.Map, X, Y, true);

				if (UserList[UserIndex].flags.AdminInvisible == 0) {
					WriteConsoleMsg(tUser,
							UserList[UserIndex].Name + " se ha trasportado hacia donde te encuentras.",
							FontTypeNames_FONTTYPE_INFO);
					FlushBuffer(tUser);
				}

				LogGM(UserList[UserIndex].Name,
						"/IRA " + UserName + " Mapa:" + vb6::CStr(UserList[tUser].Pos.Map) + " X:"
								+ vb6::CStr(UserList[tUser].Pos.X) + " Y:" + vb6::CStr(UserList[tUser].Pos.Y));
			}
		}
	}

	/* 'If we got here then packet is complete, copy data back to original queue */


}

/* '' */
/* ' Handles the "Invisible" message. */
/* ' */
/* ' @param    userIndex The index of the user sending the message. */

void HandleInvisible(int UserIndex) {
	/* '*************************************************** */
	/* 'Author: Juan Martín Sotuyo Dodero (Maraxus) */
	/* 'Last Modification: 05/17/06 */
	/* ' */
	/* '*************************************************** */
	/* 'Remove packet ID */
	UserList[UserIndex].sockctx->incomingData->ReadByte();

	if (UserTieneAlgunPrivilegios(UserIndex, PlayerType_User)) {
		return;
	}

	DoAdminInvisible(UserIndex);
	LogGM(UserList[UserIndex].Name, "/INVISIBLE");
}

/* '' */
/* ' Handles the "GMPanel" message. */
/* ' */
/* ' @param    userIndex The index of the user sending the message. */

void HandleGMPanel(int UserIndex) {
	/* '*************************************************** */
	/* 'Author: Juan Martín Sotuyo Dodero (Maraxus) */
	/* 'Last Modification: 05/17/06 */
	/* ' */
	/* '*************************************************** */
	/* 'Remove packet ID */
	UserList[UserIndex].sockctx->incomingData->ReadByte();

	if (UserTieneAlgunPrivilegios(UserIndex, PlayerType_User)) {
		return;
	}

	WriteShowGMPanelForm(UserIndex);
}

/* '' */
/* ' Handles the "GMPanel" message. */
/* ' */
/* ' @param    userIndex The index of the user sending the message. */

void HandleRequestUserList(int UserIndex) {
	/* '*************************************************** */
	/* 'Author: Juan Martín Sotuyo Dodero (Maraxus) */
	/* 'Last Modification: 01/09/07 */
	/* 'Last modified by: Lucas Tavolaro Ortiz (Tavo) */
	/* 'I haven`t found a solution to split, so i make an array of names */
	/* '*************************************************** */
	int i;
	std::vector<std::string> names;
	int Count;

	/* 'Remove packet ID */
	UserList[UserIndex].sockctx->incomingData->ReadByte();

	if (UserTieneAlgunPrivilegios(UserIndex, PlayerType_User, PlayerType_RoleMaster)) {
		return;
	}

	names.resize(0);
	names.resize(1 + LastUser);
	Count = 1;

	for (i = (1); i <= (LastUser); i++) {
		if ((vb6::LenB(UserList[i].Name) != 0)) {
			if (UserTieneAlgunPrivilegios(i, PlayerType_User)) {
				names[Count] = UserList[i].Name;
				Count = Count + 1;
			}
		}
	}

	if (Count > 1) {
		WriteUserNameList(UserIndex, names, Count - 1);
	}
}

/* '' */
/* ' Handles the "Working" message. */
/* ' */
/* ' @param    userIndex The index of the user sending the message. */

void HandleWorking(int UserIndex) {
	/* '*************************************************** */
	/* 'Author: Juan Martín Sotuyo Dodero (Maraxus) */
	/* 'Last Modification: 07/10/2010 */
	/* '07/10/2010: ZaMa - Adaptado para que funcione mas de un centinela en paralelo. */
	/* '*************************************************** */
	int i;
	std::string users;

	/* 'Remove packet ID */
	UserList[UserIndex].sockctx->incomingData->ReadByte();

	if (UserTieneAlgunPrivilegios(UserIndex, PlayerType_User, PlayerType_RoleMaster)) {
		return;
	}

	for (i = (1); i <= (LastUser); i++) {
		if (UserList[i].flags.UserLogged && UserList[i].Counters.Trabajando > 0) {
			users = users + ", " + UserList[i].Name;

			/* ' Display the user being checked by the centinel */
			if (UserList[i].flags.CentinelaIndex != 0) {
				users = users + " (*)";
			}
		}
	}

	if (vb6::LenB(users) != 0) {
		users = vb6::Right(users, vb6::Len(users) - 2);
		WriteConsoleMsg(UserIndex, "Usuarios trabajando: " + users, FontTypeNames_FONTTYPE_INFO);
	} else {
		WriteConsoleMsg(UserIndex, "No hay usuarios trabajando.", FontTypeNames_FONTTYPE_INFO);
	}
}

/* '' */
/* ' Handles the "Hiding" message. */
/* ' */
/* ' @param    userIndex The index of the user sending the message. */

void HandleHiding(int UserIndex) {
	/* '*************************************************** */
	/* 'Author: Juan Martín Sotuyo Dodero (Maraxus) */
	/* 'Last Modification: 05/17/06 */
	/* ' */
	/* '*************************************************** */
	int i;
	std::string users;

	/* 'Remove packet ID */
	UserList[UserIndex].sockctx->incomingData->ReadByte();

	if (UserTieneAlgunPrivilegios(UserIndex, PlayerType_User, PlayerType_RoleMaster)) {
		return;
	}

	for (i = (1); i <= (LastUser); i++) {
		if ((vb6::LenB(UserList[i].Name) != 0) && UserList[i].Counters.Ocultando > 0) {
			users = users + UserList[i].Name + ", ";
		}
	}

	if (vb6::LenB(users) != 0) {
		users = vb6::Left(users, vb6::Len(users) - 2);
		WriteConsoleMsg(UserIndex, "Usuarios ocultandose: " + users, FontTypeNames_FONTTYPE_INFO);
	} else {
		WriteConsoleMsg(UserIndex, "No hay usuarios ocultandose.", FontTypeNames_FONTTYPE_INFO);
	}
}

/* '' */
/* ' Handles the "Jail" message. */
/* ' */
/* ' @param    userIndex The index of the user sending the message. */

void HandleJail(int UserIndex) {
	/* '*************************************************** */
	/* 'Author: Juan Martín Sotuyo Dodero (Maraxus) */
	/* 'Last Modification: 07/06/2010 */
	/* '07/06/2010: ZaMa - Ahora no se puede usar para saber si hay dioses/admins online. */
	/* '*************************************************** */
	if (UserList[UserIndex].sockctx->incomingData->length() < 6) {
		throw insufficient_data_error();
		return;
	}

	/* 'This packet contains strings, make a copy of the data to prevent losses if it's not complete yet... */
	clsByteQueue* buffer = UserList[UserIndex].sockctx->incomingData.get();


	/* 'Remove packet ID */
	buffer->ReadByte();

	std::string UserName;
	std::string Reason;
	int jailTime;
	int Count;
	int tUser = 0;

	UserName = buffer->ReadUnicodeString();
	Reason = buffer->ReadUnicodeString();
	jailTime = buffer->ReadByte();

	if (vb6::InStr(1, UserName, "+")) {
		UserName = vb6::Replace(UserName, "+", " ");
	}

	/* '/carcel nick@motivo@<tiempo> */
	if (!UserTieneAlgunPrivilegios(UserIndex, PlayerType_RoleMaster, PlayerType_User)) {
		if (vb6::LenB(UserName) == 0 || vb6::LenB(Reason) == 0) {
			WriteConsoleMsg(UserIndex, "Utilice /carcel nick@motivo@tiempo", FontTypeNames_FONTTYPE_INFO);
		} else {
			tUser = NameIndex(UserName);

			if (tUser <= 0) {
				if ((EsDios(UserName) || EsAdmin(UserName))) {
					WriteConsoleMsg(UserIndex, "No puedes encarcelar a administradores.",
							FontTypeNames_FONTTYPE_INFO);
				} else {
					WriteConsoleMsg(UserIndex, "El usuario no está online.", FontTypeNames_FONTTYPE_INFO);
				}
			} else {
				if (!UserTieneAlgunPrivilegios(UserIndex, PlayerType_User)) {
					WriteConsoleMsg(UserIndex, "No puedes encarcelar a administradores.",
							FontTypeNames_FONTTYPE_INFO);
				} else if (jailTime > 60) {
					WriteConsoleMsg(UserIndex, "No puedés encarcelar por más de 60 minutos.",
							FontTypeNames_FONTTYPE_INFO);
				} else {
					if ((vb6::InStrB(UserName, "/") != 0)) {
						UserName = vb6::Replace(UserName, "/", "");
					}
					if ((vb6::InStrB(UserName, "/") != 0)) {
						UserName = vb6::Replace(UserName, "/", "");
					}

					auto userCharPath = GetCharPath(UserName);
					if (FileExist(userCharPath, 0)) {
						Count = vb6::val(GetVar(userCharPath, "PENAS", "Cant"));
						WriteVar(userCharPath, "PENAS", "Cant", Count + 1);
						WriteVar(userCharPath, "PENAS", "P" + vb6::CStr(Count + 1),
								vb6::LCase(UserList[UserIndex].Name) + ": CARCEL " + vb6::CStr(jailTime)
										+ "m, MOTIVO: " + vb6::LCase(Reason) + " " + vb6::CStr(vb6::Now()));
					}

					Encarcelar(tUser, jailTime, UserList[UserIndex].Name);
					LogGM(UserList[UserIndex].Name, " encarceló a " + UserName);
				}
			}
		}
	}

	/* 'If we got here then packet is complete, copy data back to original queue */


}

/* '' */
/* ' Handles the "KillNPC" message. */
/* ' */
/* ' @param    userIndex The index of the user sending the message. */

void HandleKillNPC(int UserIndex) {
	/* '*************************************************** */
	/* 'Author: Juan Martín Sotuyo Dodero (Maraxus) */
	/* 'Last Modification: 04/22/08 (NicoNZ) */
	/* ' */
	/* '*************************************************** */
	/* 'Remove packet ID */
	UserList[UserIndex].sockctx->incomingData->ReadByte();

	if (UserTieneAlgunPrivilegios(UserIndex, PlayerType_User)) {
		return;
	}

	int tNPC;
	struct npc auxNPC;

	/* 'Los consejeros no pueden RMATAr a nada en el mapa pretoriano */
	if (UserTieneAlgunPrivilegios(UserIndex, PlayerType_Consejero)) {
//       if (UserList[UserIndex].Pos.Map == MAPA_PRETORIANO) {
//       WriteConsoleMsg(UserIndex, "Los consejeros no pueden usar este comando en el mapa pretoriano.", FontTypeNames_FONTTYPE_INFO);
//       return;
//      }
	}

	tNPC = UserList[UserIndex].flags.TargetNPC;

	if (tNPC > 0) {
		WriteConsoleMsg(UserIndex, "RMatas (con posible respawn) a: " + Npclist[tNPC].Name,
				FontTypeNames_FONTTYPE_INFO);

		auxNPC = Npclist[tNPC];
		QuitarNPC(tNPC);
		ReSpawnNpc(auxNPC);

		UserList[UserIndex].flags.TargetNPC = 0;
	} else {
		WriteConsoleMsg(UserIndex, "Antes debes hacer click sobre el NPC.", FontTypeNames_FONTTYPE_INFO);
	}
}

/* '' */
/* ' Handles the "WarnUser" message. */
/* ' */
/* ' @param    userIndex The index of the user sending the message. */

void HandleWarnUser(int UserIndex) {
	/* '*************************************************** */
	/* 'Author: Nicolas Matias Gonzalez (NIGO) */
	/* 'Last Modification: 12/26/06 */
	/* ' */
	/* '*************************************************** */
	if (UserList[UserIndex].sockctx->incomingData->length() < 5) {
		throw insufficient_data_error();
		return;
	}

	/* 'This packet contains strings, make a copy of the data to prevent losses if it's not complete yet... */
	clsByteQueue* buffer = UserList[UserIndex].sockctx->incomingData.get();


	/* 'Remove packet ID */
	buffer->ReadByte();

	std::string UserName;
	std::string Reason;
	PlayerType Privs;
	int Count;

	UserName = buffer->ReadUnicodeString();
	Reason = buffer->ReadUnicodeString();

	if (!UserTieneAlgunPrivilegios(UserIndex, PlayerType_RoleMaster, PlayerType_User)) {
		if (vb6::LenB(UserName) == 0 || vb6::LenB(Reason) == 0) {
			WriteConsoleMsg(UserIndex, "Utilice /advertencia nick@motivo", FontTypeNames_FONTTYPE_INFO);
		} else {
			Privs = UserDarPrivilegioLevel(UserName);

			if (!Privs && PlayerType_User) {
				WriteConsoleMsg(UserIndex, "No puedes advertir a administradores.",
						FontTypeNames_FONTTYPE_INFO);
			} else {
				if ((vb6::InStrB(UserName, "/") != 0)) {
					UserName = vb6::Replace(UserName, "/", "");
				}
				if ((vb6::InStrB(UserName, "/") != 0)) {
					UserName = vb6::Replace(UserName, "/", "");
				}

				if (FileExist(GetCharPath(UserName), 0)) {
					Count = vb6::val(GetVar(GetCharPath(UserName), "PENAS", "Cant"));
					WriteVar(GetCharPath(UserName), "PENAS", "Cant", Count + 1);
					WriteVar(GetCharPath(UserName), "PENAS", "P" + vb6::CStr(Count + 1),
							vb6::LCase(UserList[UserIndex].Name) + ": ADVERTENCIA por: " + vb6::LCase(Reason)
									+ " " + vb6::CStr(vb6::Now()));

					WriteConsoleMsg(UserIndex, "Has advertido a " + vb6::UCase(UserName) + ".",
							FontTypeNames_FONTTYPE_INFO);
					LogGM(UserList[UserIndex].Name, " advirtio a " + UserName);
				}
			}
		}
	}

	/* 'If we got here then packet is complete, copy data back to original queue */

}

/* '' */
/* ' Handles the "EditChar" message. */
/* ' */
/* ' @param    userIndex The index of the user sending the message. */

void HandleEditChar(int UserIndex) {
	/* '*************************************************** */
	/* 'Author: Nicolas Matias Gonzalez (NIGO) */
	/* 'Last Modification: 18/09/2010 */
	/* '02/03/2009: ZaMa - Cuando editas nivel, chequea si el pj puede permanecer en clan faccionario */
	/* '11/06/2009: ZaMa - Todos los comandos se pueden usar aunque el pj este offline */
	/* '18/09/2010: ZaMa - Ahora se puede editar la vida del propio pj (cualquier rm o dios). */
	/* '*************************************************** */
	if (UserList[UserIndex].sockctx->incomingData->length() < 8) {
		throw insufficient_data_error();
		return;
	}

	/* 'This packet contains strings, make a copy of the data to prevent losses if it's not complete yet... */
	clsByteQueue* buffer = UserList[UserIndex].sockctx->incomingData.get();


	/* 'Remove packet ID */
	buffer->ReadByte();

	std::string UserName;
	int tUser = 0;
	int opcion;
	std::string Arg1;
	std::string Arg2;
	bool valido = false;
	int LoopC;
	std::string CommandString;
	std::string UserCharPath;
	int Var;

	UserName = vb6::Replace(buffer->ReadUnicodeString(), "+", " ");

	if (vb6::UCase(UserName) == "YO") {
		tUser = UserIndex;
	} else {
		tUser = NameIndex(UserName);
	}

	opcion = buffer->ReadByte();
	Arg1 = buffer->ReadUnicodeString();
	Arg2 = buffer->ReadUnicodeString();

	if (UserTieneAlgunPrivilegios(UserIndex, PlayerType_RoleMaster)) {
		if (UserTieneAlgunPrivilegios(UserIndex, PlayerType_Consejero)) {
			/* ' Los RMs consejeros sólo se pueden editar su head, body, level y vida */
			valido = tUser == UserIndex
					&& (opcion == eEditOptions_eo_Body || opcion == eEditOptions_eo_Head
							|| opcion == eEditOptions_eo_Level || opcion == eEditOptions_eo_Vida);

		} else if (UserTieneAlgunPrivilegios(UserIndex, PlayerType_SemiDios)) {
			/* ' Los RMs sólo se pueden editar su level o vida y el head y body de cualquiera */
			valido = ((opcion == eEditOptions_eo_Level || opcion == eEditOptions_eo_Vida)
					&& tUser == UserIndex) || opcion == eEditOptions_eo_Body
					|| opcion == eEditOptions_eo_Head;

		} else if (UserTieneAlgunPrivilegios(UserIndex, PlayerType_Dios)) {
			/* ' Los DRMs pueden aplicar los siguientes comandos sobre cualquiera */
			/* ' pero si quiere modificar el level o vida sólo lo puede hacer sobre sí mismo */
			valido = ((opcion == eEditOptions_eo_Level || opcion == eEditOptions_eo_Vida)
					&& tUser == UserIndex) || opcion == eEditOptions_eo_Body || opcion == eEditOptions_eo_Head
					|| opcion == eEditOptions_eo_CiticensKilled || opcion == eEditOptions_eo_CriminalsKilled
					|| opcion == eEditOptions_eo_Class || opcion == eEditOptions_eo_Skills
					|| opcion == eEditOptions_eo_addGold;
		}

		/* 'Si no es RM debe ser dios para poder usar este comando */
	} else if (UserTieneAlgunPrivilegios(UserIndex, PlayerType_Admin, PlayerType_Dios)) {

		if (opcion == eEditOptions_eo_Vida) {
			/* '  Por ahora dejo para que los dioses no puedan editar la vida de otros */
			valido = (tUser == UserIndex);
		} else {
			valido = true;
		}

	} else if (UserTieneAlgunPrivilegios(UserIndex, PlayerType_SemiDios)) {

		valido =
				(opcion == eEditOptions_eo_Poss || ((opcion == eEditOptions_eo_Vida) && (tUser == UserIndex)));

		if (UserList[UserIndex].flags.PrivEspecial) {
			valido = valido || (opcion == eEditOptions_eo_CiticensKilled)
					|| (opcion == eEditOptions_eo_CriminalsKilled);
		}

	} else if (UserTieneAlgunPrivilegios(UserIndex, PlayerType_Consejero)) {
		valido = ((opcion == eEditOptions_eo_Vida) && (tUser == UserIndex));
	}

	if (valido) {
		UserCharPath = GetCharPath(UserName);
		if (tUser <= 0 && !FileExist(UserCharPath)) {
			WriteConsoleMsg(UserIndex, "Estás intentando editar un usuario inexistente.",
					FontTypeNames_FONTTYPE_INFO);
			LogGM(UserList[UserIndex].Name, "Intentó editar un usuario inexistente.");
		} else {
			/* 'For making the Log */
			CommandString = "/MOD ";

			switch (opcion) {
			case eEditOptions_eo_Gold:
				if (vb6::val(Arg1) <= MAX_ORO_EDIT) {
					/* ' Esta offline? */
					if (tUser <= 0) {
						WriteVar(UserCharPath, "STATS", "GLD", vb6::val(Arg1));
						WriteConsoleMsg(UserIndex, "Charfile Alterado: " + UserName,
								FontTypeNames_FONTTYPE_INFO);
						/* ' Online */
					} else {
						UserList[tUser].Stats.GLD = vb6::val(Arg1);
						WriteUpdateGold(tUser);
					}
				} else {
					WriteConsoleMsg(UserIndex,
							"No está permitido utilizar valores mayores a " + vb6::CStr(MAX_ORO_EDIT)
									+ ". Su comando ha quedado en los logs del juego.",
							FontTypeNames_FONTTYPE_INFO);
				}

				/* ' Log it */
				CommandString = CommandString + "ORO ";

				break;

			case eEditOptions_eo_Experience:
			{
				int Arg1int = vb6::val(Arg1);
				if (Arg1int > 20000000) {
					Arg1int = 20000000;
				}

				/* ' Offline */
				if (tUser <= 0) {
					Var = vb6::CInt(GetVar(UserCharPath, "STATS", "EXP"));
					WriteVar(UserCharPath, "STATS", "EXP", Var + Arg1int);
					WriteConsoleMsg(UserIndex, "Charfile Alterado: " + UserName, FontTypeNames_FONTTYPE_INFO);
					/* ' Online */
				} else {
					UserList[tUser].Stats.Exp = UserList[tUser].Stats.Exp + (Arg1int);
					CheckUserLevel(tUser);
					WriteUpdateExp(tUser);
				}

				/* ' Log it */
				CommandString = CommandString + "EXP ";
			}
				break;

			case eEditOptions_eo_Body:
				if (tUser <= 0) {
					WriteVar(UserCharPath, "INIT", "Body", Arg1);
					WriteConsoleMsg(UserIndex, "Charfile Alterado: " + UserName, FontTypeNames_FONTTYPE_INFO);
				} else {
					ChangeUserChar(tUser, vb6::val(Arg1), UserList[tUser].Char.Head,
							UserList[tUser].Char.heading, UserList[tUser].Char.WeaponAnim,
							UserList[tUser].Char.ShieldAnim, UserList[tUser].Char.CascoAnim);
				}

				/* ' Log it */
				CommandString = CommandString + "BODY ";

				break;

			case eEditOptions_eo_Head:
				if (tUser <= 0) {
					WriteVar(UserCharPath, "INIT", "Head", Arg1);
					WriteConsoleMsg(UserIndex, "Charfile Alterado: " + UserName, FontTypeNames_FONTTYPE_INFO);
				} else {
					ChangeUserChar(tUser, UserList[tUser].Char.body, vb6::val(Arg1),
							UserList[tUser].Char.heading, UserList[tUser].Char.WeaponAnim,
							UserList[tUser].Char.ShieldAnim, UserList[tUser].Char.CascoAnim);
				}

				/* ' Log it */
				CommandString = CommandString + "HEAD ";

				break;

			case eEditOptions_eo_CriminalsKilled:
				Var = vb6::IIf(vb6::val(Arg1) > MAXUSERMATADOS, (double)MAXUSERMATADOS, vb6::val(Arg1));

				/* ' Offline */
				if (tUser <= 0) {
					WriteVar(UserCharPath, "FACCIONES", "CrimMatados", Var);
					WriteConsoleMsg(UserIndex, "Charfile Alterado: " + UserName, FontTypeNames_FONTTYPE_INFO);
					/* ' Online */
				} else {
					UserList[tUser].Faccion.CriminalesMatados = Var;
				}

				/* ' Log it */
				CommandString = CommandString + "CRI ";

				break;

			case eEditOptions_eo_CiticensKilled:
				Var = vb6::IIf(vb6::val(Arg1) > MAXUSERMATADOS, (double)MAXUSERMATADOS, vb6::val(Arg1));

				/* ' Offline */
				if (tUser <= 0) {
					WriteVar(UserCharPath, "FACCIONES", "CiudMatados", Var);
					WriteConsoleMsg(UserIndex, "Charfile Alterado: " + UserName, FontTypeNames_FONTTYPE_INFO);
					/* ' Online */
				} else {
					UserList[tUser].Faccion.CiudadanosMatados = Var;
				}

				/* ' Log it */
				CommandString = CommandString + "CIU ";

				break;

			case eEditOptions_eo_Level:
				if (vb6::val(Arg1) > STAT_MAXELV) {
					Arg1 = vb6::CStr(STAT_MAXELV);
					WriteConsoleMsg(UserIndex, "No puedes tener un nivel superior a " + vb6::CStr(STAT_MAXELV) + ".",
							FontTypeNames_FONTTYPE_INFO);
				}

				/* ' Chequeamos si puede permanecer en el clan */
				if (vb6::val(Arg1) >= 25) {

					int GI;
					if (tUser <= 0) {
						GI = vb6::CInt(GetVar(UserCharPath, "GUILD", "GUILDINDEX"));
					} else {
						GI = UserList[tUser].GuildIndex;
					}

					if (GI > 0) {
						if (GuildAlignment(GI) == "Del Mal" || GuildAlignment(GI) == "Real") {
							/* 'We get here, so guild has factionary alignment, we have to expulse the user */
							m_EcharMiembroDeClan(-1, UserName);

							SendData(SendTarget_ToGuildMembers, GI,
									PrepareMessageConsoleMsg(UserName + " deja el clan.",
											FontTypeNames_FONTTYPE_GUILD));
							/* ' Si esta online le avisamos */
							if (tUser > 0) {
								WriteConsoleMsg(tUser,
										"¡Ya tienes la madurez suficiente como para decidir bajo que estandarte pelearás! Por esta razón, hasta tanto no te enlistes en la facción bajo la cual tu clan está alineado, estarás excluído del mismo.",
										FontTypeNames_FONTTYPE_GUILD);
							}
						}
					}
				}

				/* ' Offline */
				if (tUser <= 0) {
					WriteVar(UserCharPath, "STATS", "ELV", vb6::val(Arg1));
					WriteConsoleMsg(UserIndex, "Charfile Alterado: " + UserName, FontTypeNames_FONTTYPE_INFO);
					/* ' Online */
				} else {
					UserList[tUser].Stats.ELV = vb6::val(Arg1);
					WriteUpdateUserStats(tUser);
				}

				/* ' Log it */
				CommandString = CommandString + "LEVEL ";

				break;

			case eEditOptions_eo_Class:
				for (LoopC = (1); LoopC <= (NUMCLASES); LoopC++) {
					if (vb6::UCase(ListaClases[LoopC]) == vb6::UCase(Arg1)) {
						break;
					}
				}

				if (LoopC > NUMCLASES) {
					WriteConsoleMsg(UserIndex, "Clase desconocida. Intente nuevamente.",
							FontTypeNames_FONTTYPE_INFO);
				} else {
					/* ' Offline */
					if (tUser <= 0) {
						WriteVar(UserCharPath, "INIT", "Clase", LoopC);
						WriteConsoleMsg(UserIndex, "Charfile Alterado: " + UserName,
								FontTypeNames_FONTTYPE_INFO);
						/* ' Online */
					} else {
						UserList[tUser].clase = static_cast<eClass>(LoopC);
					}
				}

				/* ' Log it */
				CommandString = CommandString + "CLASE ";

				break;

			case eEditOptions_eo_Skills:
				for (LoopC = (1); LoopC <= (NUMSKILLS); LoopC++) {
					if (vb6::UCase(vb6::Replace(SkillsNames[LoopC], " ", "+")) == vb6::UCase(Arg1)) {
						break;
					}
				}

				if (LoopC > NUMSKILLS) {
					WriteConsoleMsg(UserIndex, "Skill Inexistente!", FontTypeNames_FONTTYPE_INFO);
				} else {
					/* ' Offline */
					if (tUser <= 0) {
						WriteVar(UserCharPath, "Skills", "SK" + vb6::CStr(LoopC), Arg2);
						WriteVar(UserCharPath, "Skills", "EXPSK" + vb6::CStr(LoopC), 0);

						if (vb6::CInt(Arg2) < MAXSKILLPOINTS) {
							WriteVar(UserCharPath, "Skills", "ELUSK" + vb6::CStr(LoopC),
									ELU_SKILL_INICIAL * std::pow(1.05, vb6::CInt(Arg2)));
						} else {
							WriteVar(UserCharPath, "Skills", "ELUSK" + vb6::CStr(LoopC), 0);
						}

						WriteConsoleMsg(UserIndex, "Charfile Alterado: " + UserName,
								FontTypeNames_FONTTYPE_INFO);
						/* ' Online */
					} else {
						UserList[tUser].Stats.UserSkills[LoopC] = vb6::val(Arg2);
						CheckEluSkill(tUser, LoopC, true);
					}
				}

				/* ' Log it */
				CommandString = CommandString + "SKILLS ";

				break;

			case eEditOptions_eo_SkillPointsLeft:
				/* ' Offline */
				if (tUser <= 0) {
					WriteVar(UserCharPath, "STATS", "SkillPtsLibres", Arg1);
					WriteConsoleMsg(UserIndex, "Charfile Alterado: " + UserName, FontTypeNames_FONTTYPE_INFO);
					/* ' Online */
				} else {
					UserList[tUser].Stats.SkillPts = vb6::val(Arg1);
				}

				/* ' Log it */
				CommandString = CommandString + "SKILLSLIBRES ";

				break;

			case eEditOptions_eo_Nobleza:
				Var = vb6::IIf(vb6::val(Arg1) > MAXREP, (double)MAXREP, vb6::val(Arg1));

				/* ' Offline */
				if (tUser <= 0) {
					WriteVar(UserCharPath, "REP", "Nobles", Var);
					WriteConsoleMsg(UserIndex, "Charfile Alterado: " + UserName, FontTypeNames_FONTTYPE_INFO);
					/* ' Online */
				} else {
					UserList[tUser].Reputacion.NobleRep = Var;
				}

				/* ' Log it */
				CommandString = CommandString + "NOB ";

				break;

			case eEditOptions_eo_Asesino:
				Var = vb6::IIf(vb6::val(Arg1) > MAXREP, (double)MAXREP, vb6::val(Arg1));

				/* ' Offline */
				if (tUser <= 0) {
					WriteVar(UserCharPath, "REP", "Asesino", Var);
					WriteConsoleMsg(UserIndex, "Charfile Alterado: " + UserName, FontTypeNames_FONTTYPE_INFO);
					/* ' Online */
				} else {
					UserList[tUser].Reputacion.AsesinoRep = Var;
				}

				/* ' Log it */
				CommandString = CommandString + "ASE ";

				break;

			case eEditOptions_eo_Sex:
				int Sex;
				/* ' Mujer? */
				Sex = vb6::IIf(vb6::UCase(Arg1) == "MUJER", (int) eGenero_Mujer, 0);
				/* ' Hombre? */
				Sex = vb6::IIf(vb6::UCase(Arg1) == "HOMBRE", (int) eGenero_Hombre, Sex);

				/* ' Es Hombre o mujer? */
				if (Sex != 0) {
					/* ' OffLine */
					if (tUser <= 0) {
						WriteVar(UserCharPath, "INIT", "Genero", Sex);
						WriteConsoleMsg(UserIndex, "Charfile Alterado: " + UserName,
								FontTypeNames_FONTTYPE_INFO);
						/* ' Online */
					} else {
						UserList[tUser].Genero = static_cast<eGenero>(Sex);
					}
				} else {
					WriteConsoleMsg(UserIndex, "Genero desconocido. Intente nuevamente.",
							FontTypeNames_FONTTYPE_INFO);
				}

				/* ' Log it */
				CommandString = CommandString + "SEX ";

				break;

			case eEditOptions_eo_Raza:
				int raza;

				Arg1 = vb6::UCase(Arg1);
				if (Arg1 == "HUMANO") {
					raza = eRaza_Humano;
				} else if (Arg1 == "ELFO") {
					raza = eRaza_Elfo;
				} else if (Arg1 == "DROW") {
					raza = eRaza_Drow;
				} else if (Arg1 == "ENANO") {
					raza = eRaza_Enano;
				} else if (Arg1 == "GNOMO") {
					raza = eRaza_Gnomo;
				} else {
					raza = 0;
				}

				if (raza == 0) {
					WriteConsoleMsg(UserIndex, "Raza desconocida. Intente nuevamente.",
							FontTypeNames_FONTTYPE_INFO);
				} else {
					if (tUser <= 0) {
						WriteVar(UserCharPath, "INIT", "Raza", raza);
						WriteConsoleMsg(UserIndex, "Charfile Alterado: " + UserName,
								FontTypeNames_FONTTYPE_INFO);
					} else {
						UserList[tUser].raza = static_cast<eRaza>(raza);
					}
				}

				/* ' Log it */
				CommandString = CommandString + "RAZA ";
				break;

			case eEditOptions_eo_addGold:

				int bankGold;

				if (vb6::Abs(vb6::CInt(Arg1)) > MAX_ORO_EDIT) {
					WriteConsoleMsg(UserIndex,
							"No está permitido utilizar valores mayores a " + vb6::CStr(MAX_ORO_EDIT) + ".",
							FontTypeNames_FONTTYPE_INFO);
				} else {
					if (tUser <= 0) {
						bankGold = vb6::CInt(GetVar(GetCharPath(UserName), "STATS", "BANCO"));
						WriteVar(UserCharPath, "STATS", "BANCO",
								vb6::IIf(bankGold + vb6::val(Arg1) <= 0, 0.0, bankGold + vb6::val(Arg1)));
						WriteConsoleMsg(UserIndex,
								"Se le ha agregado " + Arg1 + " monedas de oro a " + UserName + ".",
								FontTypeNames_FONTTYPE_TALK);
					} else {
						UserList[tUser].Stats.Banco = vb6::IIf(
								UserList[tUser].Stats.Banco + vb6::val(Arg1) <= 0, 0.0,
								UserList[tUser].Stats.Banco + vb6::val(Arg1));
						WriteConsoleMsg(tUser, STANDARD_BOUNTY_HUNTER_MESSAGE, FontTypeNames_FONTTYPE_TALK);
					}
				}

				/* ' Log it */
				CommandString = CommandString + "AGREGAR ";

				break;

			case eEditOptions_eo_Vida:

				if (vb6::val(Arg1) > MAX_VIDA_EDIT) {
					Arg1 = vb6::CStr(MAX_VIDA_EDIT);
					WriteConsoleMsg(UserIndex, "No puedes tener vida superior a " + vb6::CStr(MAX_VIDA_EDIT) + ".",
							FontTypeNames_FONTTYPE_INFO);
				}

				/* ' No valido si esta offline, porque solo se puede editar a si mismo */
				UserList[tUser].Stats.MaxHp = vb6::val(Arg1);
				UserList[tUser].Stats.MinHp = vb6::val(Arg1);

				WriteUpdateUserStats(tUser);

				/* ' Log it */
				CommandString = CommandString + "VIDA ";

				break;

			case eEditOptions_eo_Poss:

				int Map;
				int X;
				int Y;

				Map = vb6::val(ReadField(1, Arg1, 45));
				X = vb6::val(ReadField(2, Arg1, 45));
				Y = vb6::val(ReadField(3, Arg1, 45));

				if (InMapBounds(Map, X, Y)) {

					if (tUser <= 0) {
						WriteVar(UserCharPath, "INIT", "POSITION", vb6::string_format("%d-%d-%d", Map, X, Y));
						WriteConsoleMsg(UserIndex, "Charfile Alterado: " + UserName,
								FontTypeNames_FONTTYPE_INFO);
					} else {
						WarpUserChar(tUser, Map, X, Y, true, true);
						WriteConsoleMsg(UserIndex, "Usuario teletransportado: " + UserName,
								FontTypeNames_FONTTYPE_INFO);
					}
				} else {
					WriteConsoleMsg(UserIndex, "Posición inválida", FontTypeNames_FONTTYPE_INFO);
				}

				/* ' Log it */
				CommandString = CommandString + "POSS ";

				break;

			default:
				WriteConsoleMsg(UserIndex, "Comando no permitido.", FontTypeNames_FONTTYPE_INFO);
				CommandString = CommandString + "UNKOWN ";

				break;
			}

			CommandString = CommandString + Arg1 + " " + Arg2;
			LogGM(UserList[UserIndex].Name, CommandString + " " + UserName);

		}
	}

	/* 'If we got here then packet is complete, copy data back to original queue */


}

/* '' */
/* ' Handles the "RequestCharInfo" message. */
/* ' */
/* ' @param    userIndex The index of the user sending the message. */

void HandleRequestCharInfo(int UserIndex) {
	/* '*************************************************** */
	/* 'Author: Fredy Horacio Treboux (liquid) */
	/* 'Last Modification: 01/08/07 */
	/* 'Last Modification by: (liquid).. alto bug zapallo.. */
	/* '*************************************************** */
	if (UserList[UserIndex].sockctx->incomingData->length() < 3) {
		throw insufficient_data_error();
		return;
	}

	/* 'This packet contains strings, make a copy of the data to prevent losses if it's not complete yet... */
	clsByteQueue* buffer = UserList[UserIndex].sockctx->incomingData.get();


	/* 'Remove packet ID */
	buffer->ReadByte();

	std::string TargetName;
	int TargetIndex;

	TargetName = vb6::Replace(buffer->ReadUnicodeString(), "+", " ");
	TargetIndex = NameIndex(TargetName);

	if (UserTieneAlgunPrivilegios(UserIndex, PlayerType_Admin, PlayerType_Dios, PlayerType_SemiDios)) {
		/* 'is the player offline? */
		if (TargetIndex <= 0) {
			/* 'don't allow to retrieve administrator's info */
			if (!(EsDios(TargetName) || EsAdmin(TargetName))) {
				WriteConsoleMsg(UserIndex, "Usuario offline, buscando en charfile.",
						FontTypeNames_FONTTYPE_INFO);
				SendUserStatsTxtOFF(UserIndex, TargetName);
			}
		} else {
			/* 'don't allow to retrieve administrator's info */
			if (UserTieneAlgunPrivilegios(TargetIndex, PlayerType_User, PlayerType_Consejero, PlayerType_SemiDios)) {
				SendUserStatsTxt(UserIndex, TargetIndex);
			}
		}
	}

	/* 'If we got here then packet is complete, copy data back to original queue */


}

/* '' */
/* ' Handles the "RequestCharStats" message. */
/* ' */
/* ' @param    userIndex The index of the user sending the message. */

void HandleRequestCharStats(int UserIndex) {
	/* '*************************************************** */
	/* 'Author: Nicolas Matias Gonzalez (NIGO) */
	/* 'Last Modification: 07/06/2010 */
	/* '07/06/2010: ZaMa - Ahora no se puede usar para saber si hay dioses/admins online. */
	/* '*************************************************** */
	if (UserList[UserIndex].sockctx->incomingData->length() < 3) {
		throw insufficient_data_error();
		return;
	}

	/* 'This packet contains strings, make a copy of the data to prevent losses if it's not complete yet... */
	clsByteQueue* buffer = UserList[UserIndex].sockctx->incomingData.get();


	/* 'Remove packet ID */
	buffer->ReadByte();

	std::string UserName;
	int tUser = 0;

	bool UserIsAdmin = false;
	bool OtherUserIsAdmin = false;

	UserName = buffer->ReadUnicodeString();

	UserIsAdmin = UserTieneAlgunPrivilegios(UserIndex, PlayerType_Admin, PlayerType_Dios);

	if (!UserTieneAlgunPrivilegios(UserIndex, PlayerType_RoleMaster)
			&& (UserTieneAlgunPrivilegios(UserIndex, PlayerType_SemiDios) || UserIsAdmin)) {
		LogGM(UserList[UserIndex].Name, "/STAT " + UserName);

		tUser = NameIndex(UserName);

		OtherUserIsAdmin = EsDios(UserName) || EsAdmin(UserName);

		if (tUser <= 0) {
			if (UserIsAdmin || !OtherUserIsAdmin) {
				WriteConsoleMsg(UserIndex, "Usuario offline. Leyendo charfile... ",
						FontTypeNames_FONTTYPE_INFO);

				SendUserMiniStatsTxtFromChar(UserIndex, UserName);
			} else {
				WriteConsoleMsg(UserIndex, "No puedes ver los stats de un dios o admin.",
						FontTypeNames_FONTTYPE_INFO);
			}
		} else {
			if (UserIsAdmin || !OtherUserIsAdmin) {
				SendUserMiniStatsTxt(UserIndex, tUser);
			} else {
				WriteConsoleMsg(UserIndex, "No puedes ver los stats de un dios o admin.",
						FontTypeNames_FONTTYPE_INFO);
			}
		}
	}

	/* 'If we got here then packet is complete, copy data back to original queue */


}

/* '' */
/* ' Handles the "RequestCharGold" message. */
/* ' */
/* ' @param    userIndex The index of the user sending the message. */

void HandleRequestCharGold(int UserIndex) {
	/* '*************************************************** */
	/* 'Author: Nicolas Matias Gonzalez (NIGO) */
	/* 'Last Modification: 07/06/2010 */
	/* '07/06/2010: ZaMa - Ahora no se puede usar para saber si hay dioses/admins online. */
	/* '*************************************************** */
	if (UserList[UserIndex].sockctx->incomingData->length() < 3) {
		throw insufficient_data_error();
		return;
	}

	/* 'This packet contains strings, make a copy of the data to prevent losses if it's not complete yet... */
	clsByteQueue* buffer = UserList[UserIndex].sockctx->incomingData.get();


	/* 'Remove packet ID */
	buffer->ReadByte();

	std::string UserName;
	int tUser = 0;

	bool UserIsAdmin = false;
	bool OtherUserIsAdmin = false;

	UserName = buffer->ReadUnicodeString();

	UserIsAdmin = (UserTieneAlgunPrivilegios(UserIndex, PlayerType_Admin, PlayerType_Dios));

	if (UserTieneAlgunPrivilegios(UserIndex, PlayerType_SemiDios) || UserIsAdmin) {

		LogGM(UserList[UserIndex].Name, "/BAL " + UserName);

		tUser = NameIndex(UserName);
		OtherUserIsAdmin = EsDios(UserName) || EsAdmin(UserName);

		tUser = NameIndex(UserName);
		OtherUserIsAdmin = EsDios(UserName) || EsAdmin(UserName);

		if (tUser <= 0) {
			if (UserIsAdmin || !OtherUserIsAdmin) {
				WriteConsoleMsg(UserIndex, "Usuario offline. Leyendo charfile... ",
						FontTypeNames_FONTTYPE_TALK);

				SendUserOROTxtFromChar(UserIndex, UserName);
			} else {
				WriteConsoleMsg(UserIndex, "No puedes ver el oro de un dios o admin.",
						FontTypeNames_FONTTYPE_INFO);
			}
		} else {
			if (UserIsAdmin || !OtherUserIsAdmin) {
				WriteConsoleMsg(UserIndex,
						"El usuario " + UserName + " tiene " + vb6::CStr(UserList[tUser].Stats.Banco) + " en el banco.",
						FontTypeNames_FONTTYPE_TALK);
			} else {
				WriteConsoleMsg(UserIndex, "No puedes ver el oro de un dios o admin.",
						FontTypeNames_FONTTYPE_INFO);
			}
		}
	}

	/* 'If we got here then packet is complete, copy data back to original queue */


}

/* '' */
/* ' Handles the "RequestCharInventory" message. */
/* ' */
/* ' @param    userIndex The index of the user sending the message. */

void HandleRequestCharInventory(int UserIndex) {
	/* '*************************************************** */
	/* 'Author: Nicolas Matias Gonzalez (NIGO) */
	/* 'Last Modification: 07/06/2010 */
	/* '07/06/2010: ZaMa - Ahora no se puede usar para saber si hay dioses/admins online. */
	/* '*************************************************** */
	if (UserList[UserIndex].sockctx->incomingData->length() < 3) {
		throw insufficient_data_error();
		return;
	}

	/* 'This packet contains strings, make a copy of the data to prevent losses if it's not complete yet... */
	clsByteQueue* buffer = UserList[UserIndex].sockctx->incomingData.get();


	/* 'Remove packet ID */
	buffer->ReadByte();

	std::string UserName;
	int tUser = 0;

	bool UserIsAdmin = false;
	bool OtherUserIsAdmin = false;

	UserName = buffer->ReadUnicodeString();

	UserIsAdmin = (UserTieneAlgunPrivilegios(UserIndex, PlayerType_Admin, PlayerType_Dios));

	if (UserTieneAlgunPrivilegios(UserIndex, PlayerType_Admin, PlayerType_Dios, PlayerType_SemiDios)) {
		LogGM(UserList[UserIndex].Name, "/INV " + UserName);

		tUser = NameIndex(UserName);
		OtherUserIsAdmin = EsDios(UserName) || EsAdmin(UserName);

		tUser = NameIndex(UserName);
		OtherUserIsAdmin = EsDios(UserName) || EsAdmin(UserName);

		if (tUser <= 0) {
			if (UserIsAdmin || !OtherUserIsAdmin) {
				WriteConsoleMsg(UserIndex, "Usuario offline. Leyendo del charfile...",
						FontTypeNames_FONTTYPE_TALK);

				SendUserInvTxtFromChar(UserIndex, UserName);
			} else {
				WriteConsoleMsg(UserIndex, "No puedes ver el inventario de un dios o admin.",
						FontTypeNames_FONTTYPE_INFO);
			}
		} else {
			if (UserIsAdmin || !OtherUserIsAdmin) {
				SendUserInvTxt(UserIndex, tUser);
			} else {
				WriteConsoleMsg(UserIndex, "No puedes ver el inventario de un dios o admin.",
						FontTypeNames_FONTTYPE_INFO);
			}
		}
	}

	/* 'If we got here then packet is complete, copy data back to original queue */


}

/* '' */
/* ' Handles the "RequestCharBank" message. */
/* ' */
/* ' @param    userIndex The index of the user sending the message. */

void HandleRequestCharBank(int UserIndex) {
	/* '*************************************************** */
	/* 'Author: Nicolas Matias Gonzalez (NIGO) */
	/* 'Last Modification: 07/06/2010 */
	/* '07/06/2010: ZaMa - Ahora no se puede usar para saber si hay dioses/admins online. */
	/* '*************************************************** */
	if (UserList[UserIndex].sockctx->incomingData->length() < 3) {
		throw insufficient_data_error();
		return;
	}

	/* 'This packet contains strings, make a copy of the data to prevent losses if it's not complete yet... */
	clsByteQueue* buffer = UserList[UserIndex].sockctx->incomingData.get();


	/* 'Remove packet ID */
	buffer->ReadByte();

	std::string UserName;
	int tUser = 0;

	bool UserIsAdmin = false;
	bool OtherUserIsAdmin = false;

	UserName = buffer->ReadUnicodeString();

	UserIsAdmin = (UserTieneAlgunPrivilegios(UserIndex, PlayerType_Admin, PlayerType_Dios));

	if (UserTieneAlgunPrivilegios(UserIndex, PlayerType_SemiDios) || UserIsAdmin) {
		LogGM(UserList[UserIndex].Name, "/BOV " + UserName);

		tUser = NameIndex(UserName);
		OtherUserIsAdmin = EsDios(UserName) || EsAdmin(UserName);

		tUser = NameIndex(UserName);
		OtherUserIsAdmin = EsDios(UserName) || EsAdmin(UserName);

		if (tUser <= 0) {
			if (UserIsAdmin || !OtherUserIsAdmin) {
				WriteConsoleMsg(UserIndex, "Usuario offline. Leyendo charfile... ",
						FontTypeNames_FONTTYPE_TALK);

				SendUserBovedaTxtFromChar(UserIndex, UserName);
			} else {
				WriteConsoleMsg(UserIndex, "No puedes ver la bóveda de un dios o admin.",
						FontTypeNames_FONTTYPE_INFO);
			}
		} else {
			if (UserIsAdmin || !OtherUserIsAdmin) {
				SendUserBovedaTxt(UserIndex, tUser);
			} else {
				WriteConsoleMsg(UserIndex, "No puedes ver la bóveda de un dios o admin.",
						FontTypeNames_FONTTYPE_INFO);
			}
		}
	}

	/* 'If we got here then packet is complete, copy data back to original queue */


}

/* '' */
/* ' Handles the "RequestCharSkills" message. */
/* ' */
/* ' @param    userIndex The index of the user sending the message. */

void HandleRequestCharSkills(int UserIndex) {
	/* '*************************************************** */
	/* 'Author: Nicolas Matias Gonzalez (NIGO) */
	/* 'Last Modification: 12/29/06 */
	/* ' */
	/* '*************************************************** */
	if (UserList[UserIndex].sockctx->incomingData->length() < 3) {
		throw insufficient_data_error();
		return;
	}

	/* 'This packet contains strings, make a copy of the data to prevent losses if it's not complete yet... */
	clsByteQueue* buffer = UserList[UserIndex].sockctx->incomingData.get();


	/* 'Remove packet ID */
	buffer->ReadByte();

	std::string UserName;
	int tUser = 0;
	int LoopC;
	std::string message;

	UserName = buffer->ReadUnicodeString();
	tUser = NameIndex(UserName);

	if (UserTieneAlgunPrivilegios(UserIndex, PlayerType_Admin, PlayerType_Dios, PlayerType_SemiDios)) {
		LogGM(UserList[UserIndex].Name, "/STATS " + UserName);

		if (tUser <= 0) {
			if ((vb6::InStrB(UserName, "/") != 0)) {
				UserName = vb6::Replace(UserName, "/", "");
			}
			if ((vb6::InStrB(UserName, "/") != 0)) {
				UserName = vb6::Replace(UserName, "/", "");
			}

			for (LoopC = (1); LoopC <= (NUMSKILLS); LoopC++) {
				message = message + "CHAR>" + SkillsNames[LoopC] + " = "
						+ GetVar(GetCharPath(UserName), "SKILLS", "SK" + vb6::CStr(LoopC)) + vbCrLf;
			}

			WriteConsoleMsg(UserIndex,
					message + "CHAR> Libres:"
							+ GetVar(GetCharPath(UserName), "STATS", "SKILLPTSLIBRES"),
					FontTypeNames_FONTTYPE_INFO);
		} else {
			SendUserSkillsTxt(UserIndex, tUser);
		}
	}

	/* 'If we got here then packet is complete, copy data back to original queue */


}

/* '' */
/* ' Handles the "ReviveChar" message. */
/* ' */
/* ' @param    userIndex The index of the user sending the message. */

void HandleReviveChar(int UserIndex) {
	/* '*************************************************** */
	/* 'Author: Nicolas Matias Gonzalez (NIGO) */
	/* 'Last Modification: 11/03/2010 */
	/* '11/03/2010: ZaMa - Al revivir con el comando, si esta navegando le da cuerpo e barca. */
	/* '*************************************************** */
	if (UserList[UserIndex].sockctx->incomingData->length() < 3) {
		throw insufficient_data_error();
		return;
	}

	/* 'This packet contains strings, make a copy of the data to prevent losses if it's not complete yet... */
	clsByteQueue* buffer = UserList[UserIndex].sockctx->incomingData.get();


	/* 'Remove packet ID */
	buffer->ReadByte();

	std::string UserName;
	int tUser = 0;

	UserName = buffer->ReadUnicodeString();

	if (UserTieneAlgunPrivilegios(UserIndex, PlayerType_Admin, PlayerType_Dios, PlayerType_SemiDios)) {
		if (vb6::UCase(UserName) != "YO") {
			tUser = NameIndex(UserName);
		} else {
			tUser = UserIndex;
		}

		if (tUser <= 0) {
			WriteConsoleMsg(UserIndex, "Usuario offline.", FontTypeNames_FONTTYPE_INFO);
		} else {
			/* 'If dead, show him alive (naked). */
			if (UserList[tUser].flags.Muerto == 1) {
				UserList[tUser].flags.Muerto = 0;

				if (UserList[tUser].flags.Navegando == 1) {
					ToggleBoatBody(tUser);
				} else {
					DarCuerpoDesnudo(tUser);
				}

				if (UserList[tUser].flags.Traveling == 1) {
					UserList[tUser].flags.Traveling = 0;
					UserList[tUser].Counters.goHome = 0;
					WriteMultiMessage(tUser, eMessages_CancelHome);
				}

				ChangeUserChar(tUser, UserList[tUser].Char.body, UserList[tUser].OrigChar.Head,
						UserList[tUser].Char.heading, UserList[tUser].Char.WeaponAnim,
						UserList[tUser].Char.ShieldAnim, UserList[tUser].Char.CascoAnim);

				WriteConsoleMsg(tUser, UserList[UserIndex].Name + " te ha resucitado.",
						FontTypeNames_FONTTYPE_INFO);
			} else {
				WriteConsoleMsg(tUser, UserList[UserIndex].Name + " te ha curado.",
						FontTypeNames_FONTTYPE_INFO);
			}

			UserList[tUser].Stats.MinHp = UserList[tUser].Stats.MaxHp;

			if (UserList[tUser].flags.Traveling == 1) {
				UserList[tUser].Counters.goHome = 0;
				UserList[tUser].flags.Traveling = 0;
				WriteMultiMessage(tUser, eMessages_CancelHome);
			}

			WriteUpdateHP(tUser);

			FlushBuffer(tUser);

			LogGM(UserList[UserIndex].Name, "Resucito a " + UserName);
		}
	}

	/* 'If we got here then packet is complete, copy data back to original queue */


}

/* '' */
/* ' Handles the "OnlineGM" message. */
/* ' */
/* ' @param    userIndex The index of the user sending the message. */

void HandleOnlineGM(int UserIndex) {
	/* '*************************************************** */
	/* 'Author: Fredy Horacio Treboux (liquid) */
	/* 'Last Modification: 12/28/06 */
	/* ' */
	/* '*************************************************** */
	int i;
	std::string list;
	bool esdios = false;

	/* 'Remove packet ID */
	UserList[UserIndex].sockctx->incomingData->ReadByte();

	if (UserTieneAlgunPrivilegios(UserIndex, PlayerType_User, PlayerType_Consejero)) {
		return;
	}

	if (UserTieneAlgunPrivilegios(UserIndex, PlayerType_Dios, PlayerType_Admin)) {
		esdios = true;
	}

	for (i = (1); i <= (LastUser); i++) {
		if (UserList[i].flags.UserLogged) {
			if (!UserTienePrivilegio(i, PlayerType_User)) {
				if (esdios || UserTieneMasPrivilegiosQue(UserIndex, i)) {
					list = list + UserList[i].Name + ", ";
				}
			}
		}
	}

	if (vb6::LenB(list) != 0) {
		list = vb6::Left(list, vb6::Len(list) - 2);
		WriteConsoleMsg(UserIndex, list + ".", FontTypeNames_FONTTYPE_INFO);
	} else {
		WriteConsoleMsg(UserIndex, "No hay GMs Online.", FontTypeNames_FONTTYPE_INFO);
	}
}

/* '' */
/* ' Handles the "OnlineMap" message. */
/* ' */
/* ' @param    userIndex The index of the user sending the message. */

void HandleOnlineMap(int UserIndex) {
	/* '*************************************************** */
	/* 'Author: Nicolas Matias Gonzalez (NIGO) */
	/* 'Last Modification: 23/03/2009 */
	/* '23/03/2009: ZaMa - Ahora no requiere estar en el mapa, sino que por defecto se toma en el que esta, pero se puede especificar otro */
	/* '*************************************************** */
	/* 'Remove packet ID */
	UserList[UserIndex].sockctx->incomingData->ReadByte();

	int Map;
	Map = UserList[UserIndex].sockctx->incomingData->ReadInteger();

	if (UserTieneAlgunPrivilegios(UserIndex, PlayerType_User, PlayerType_Consejero)) {
		return;
	}

	int LoopC;
	std::string list;
	bool esdios = false;

	if (UserTieneAlgunPrivilegios(UserIndex, PlayerType_Dios, PlayerType_Admin)) {
		esdios = true;
	}

	for (LoopC = (1); LoopC <= (LastUser); LoopC++) {
		if (UserList[LoopC].flags.UserLogged && UserList[LoopC].Pos.Map == Map) {
			if (esdios || UserTieneAlgunPrivilegios(LoopC, PlayerType_User, PlayerType_Consejero, PlayerType_SemiDios)) {
				list = list + UserList[LoopC].Name + ", ";
			}
		}
	}

	if (vb6::Len(list) > 2) {
		list = vb6::Left(list, vb6::Len(list) - 2);
	}

	WriteConsoleMsg(UserIndex, "Usuarios en el mapa: " + list, FontTypeNames_FONTTYPE_INFO);
	LogGM(UserList[UserIndex].Name, "/ONLINEMAP " + vb6::CStr(Map));
}

/* '' */
/* ' Handles the "Forgive" message. */
/* ' */
/* ' @param    userIndex The index of the user sending the message. */

void HandleForgive(int UserIndex) {
	/* '*************************************************** */
	/* 'Author: Nicolas Matias Gonzalez (NIGO) */
	/* 'Last Modification: 07/06/2010 */
	/* '07/06/2010: ZaMa - Ahora no se puede usar para saber si hay dioses/admins online. */
	/* '*************************************************** */
	if (UserList[UserIndex].sockctx->incomingData->length() < 3) {
		throw insufficient_data_error();
		return;
	}

	/* 'This packet contains strings, make a copy of the data to prevent losses if it's not complete yet... */
	clsByteQueue* buffer = UserList[UserIndex].sockctx->incomingData.get();


	/* 'Remove packet ID */
	buffer->ReadByte();

	std::string UserName;
	int tUser = 0;

	UserName = buffer->ReadUnicodeString();

	if (!UserTienePrivilegio(UserIndex, PlayerType_RoleMaster)
			&& UserTieneAlgunPrivilegios(UserIndex, PlayerType_Admin, PlayerType_Dios, PlayerType_SemiDios)) {
		tUser = NameIndex(UserName);

		if (tUser > 0) {
			if (EsNewbie(tUser)) {
				VolverCiudadano(tUser);
			} else {
				LogGM(UserList[UserIndex].Name, "Intento perdonar un personaje de nivel avanzado.");

				if (!(EsDios(UserName) || EsAdmin(UserName))) {
					WriteConsoleMsg(UserIndex, "Sólo se permite perdonar newbies.",
							FontTypeNames_FONTTYPE_INFO);
				}
			}
		}
	}

	/* 'If we got here then packet is complete, copy data back to original queue */


}

/* '' */
/* ' Handles the "Kick" message. */
/* ' */
/* ' @param    userIndex The index of the user sending the message. */

void HandleKick(int UserIndex) {
	/* '*************************************************** */
	/* 'Author: Nicolas Matias Gonzalez (NIGO) */
	/* 'Last Modification: 07/06/2010 */
	/* '07/06/2010: ZaMa - Ahora no se puede usar para saber si hay dioses/admins online. */
	/* '*************************************************** */
	if (UserList[UserIndex].sockctx->incomingData->length() < 3) {
		throw insufficient_data_error();
		return;
	}

	/* 'This packet contains strings, make a copy of the data to prevent losses if it's not complete yet... */
	clsByteQueue* buffer = UserList[UserIndex].sockctx->incomingData.get();


	/* 'Remove packet ID */
	buffer->ReadByte();

	std::string UserName;
	int tUser = 0;
	bool IsAdmin;

	UserName = buffer->ReadUnicodeString();
	IsAdmin = UserTieneAlgunPrivilegios(UserIndex, PlayerType_Admin, PlayerType_Dios);

	if (UserTieneAlgunPrivilegios(UserIndex, PlayerType_SemiDios) | IsAdmin) {
		tUser = NameIndex(UserName);

		if (tUser <= 0) {
			if (!(EsDios(UserName) || EsAdmin(UserName)) || IsAdmin) {
				WriteConsoleMsg(UserIndex, "El usuario no está online.", FontTypeNames_FONTTYPE_INFO);
			} else {
				WriteConsoleMsg(UserIndex, "No puedes echar a alguien con jerarquía mayor a la tuya.",
						FontTypeNames_FONTTYPE_INFO);
			}
		} else {
			if (UserTieneMasPrivilegiosQue(tUser, UserIndex)) {
				WriteConsoleMsg(UserIndex, "No puedes echar a alguien con jerarquía mayor a la tuya.",
						FontTypeNames_FONTTYPE_INFO);
			} else {
				CloseSocket(tUser);
				LogGM(UserList[UserIndex].Name, "Echó a " + UserName);
			}
		}
	}

	/* 'If we got here then packet is complete, copy data back to original queue */


}

/* '' */
/* ' Handles the "Execute" message. */
/* ' */
/* ' @param    userIndex The index of the user sending the message. */

void HandleExecute(int UserIndex) {
	/* '*************************************************** */
	/* 'Author: Nicolas Matias Gonzalez (NIGO) */
	/* 'Last Modification: 07/06/2010 */
	/* '07/06/2010: ZaMa - Ahora no se puede usar para saber si hay dioses/admins online. */
	/* '*************************************************** */
	if (UserList[UserIndex].sockctx->incomingData->length() < 3) {
		throw insufficient_data_error();
		return;
	}

	/* 'This packet contains strings, make a copy of the data to prevent losses if it's not complete yet... */
	clsByteQueue* buffer = UserList[UserIndex].sockctx->incomingData.get();


	/* 'Remove packet ID */
	buffer->ReadByte();

	std::string UserName;
	int tUser = 0;

	UserName = buffer->ReadUnicodeString();

	if (!UserTieneAlgunPrivilegios(UserIndex, PlayerType_RoleMaster)
			&& UserTieneAlgunPrivilegios(UserIndex, PlayerType_Admin, PlayerType_Dios, PlayerType_SemiDios)) {
		tUser = NameIndex(UserName);

		if (tUser > 0) {
			if (!UserTieneAlgunPrivilegios(tUser, PlayerType_User)) {
				WriteConsoleMsg(UserIndex, "¿¿Estás loco?? ¿¿Cómo vas a pinatear un gm?? :@",
						FontTypeNames_FONTTYPE_INFO);
			} else {
				UserDie(tUser);
				SendData(SendTarget_ToAll, 0,
						PrepareMessageConsoleMsg(
								UserList[UserIndex].Name + " ha ejecutado a " + UserName + ".",
								FontTypeNames_FONTTYPE_EJECUCION));
				LogGM(UserList[UserIndex].Name, " ejecuto a " + UserName);
			}
		} else {
			if (!(EsDios(UserName) || EsAdmin(UserName))) {
				WriteConsoleMsg(UserIndex, "No está online.", FontTypeNames_FONTTYPE_INFO);
			} else {
				WriteConsoleMsg(UserIndex, "¿¿Estás loco?? ¿¿Cómo vas a pinatear un gm?? :@",
						FontTypeNames_FONTTYPE_INFO);
			}
		}
	}

	/* 'If we got here then packet is complete, copy data back to original queue */


}

/* '' */
/* ' Handles the "BanChar" message. */
/* ' */
/* ' @param    userIndex The index of the user sending the message. */

void HandleBanChar(int UserIndex) {
	/* '*************************************************** */
	/* 'Author: Nicolas Matias Gonzalez (NIGO) */
	/* 'Last Modification: 12/29/06 */
	/* ' */
	/* '*************************************************** */
	if (UserList[UserIndex].sockctx->incomingData->length() < 5) {
		throw insufficient_data_error();
		return;
	}

	/* 'This packet contains strings, make a copy of the data to prevent losses if it's not complete yet... */
	clsByteQueue* buffer = UserList[UserIndex].sockctx->incomingData.get();


	/* 'Remove packet ID */
	buffer->ReadByte();

	std::string UserName;
	std::string Reason;

	UserName = buffer->ReadUnicodeString();
	Reason = buffer->ReadUnicodeString();

	if (!UserTieneAlgunPrivilegios(UserIndex, PlayerType_RoleMaster)
			&& UserTieneAlgunPrivilegios(UserIndex, PlayerType_Admin, PlayerType_Dios, PlayerType_SemiDios)) {
		BanCharacter(UserIndex, UserName, Reason);
	}

	/* 'If we got here then packet is complete, copy data back to original queue */


}

/* '' */
/* ' Handles the "UnbanChar" message. */
/* ' */
/* ' @param    userIndex The index of the user sending the message. */

void HandleUnbanChar(int UserIndex) {
	/* '*************************************************** */
	/* 'Author: Nicolas Matias Gonzalez (NIGO) */
	/* 'Last Modification: 12/29/06 */
	/* ' */
	/* '*************************************************** */
	if (UserList[UserIndex].sockctx->incomingData->length() < 3) {
		throw insufficient_data_error();
		return;
	}

	/* 'This packet contains strings, make a copy of the data to prevent losses if it's not complete yet... */
	clsByteQueue* buffer = UserList[UserIndex].sockctx->incomingData.get();


	/* 'Remove packet ID */
	buffer->ReadByte();

	std::string UserName;
	int cantPenas;

	UserName = buffer->ReadUnicodeString();

	if (!UserTieneAlgunPrivilegios(UserIndex, PlayerType_RoleMaster)
			&& UserTieneAlgunPrivilegios(UserIndex, PlayerType_Admin, PlayerType_Dios, PlayerType_SemiDios)) {
		if ((vb6::InStrB(UserName, "/") != 0)) {
			UserName = vb6::Replace(UserName, "/", "");
		}
		if ((vb6::InStrB(UserName, "/") != 0)) {
			UserName = vb6::Replace(UserName, "/", "");
		}

		if (!FileExist(GetCharPath(UserName), 0)) {
			WriteConsoleMsg(UserIndex, "Charfile inexistente (no use +).", FontTypeNames_FONTTYPE_INFO);
		} else {
			if ((vb6::val(GetVar(GetCharPath(UserName), "FLAGS", "Ban")) == 1)) {
				UnBan(UserName);

				/* 'penas */
				cantPenas = vb6::val(GetVar(GetCharPath(UserName), "PENAS", "Cant"));
				WriteVar(GetCharPath(UserName), "PENAS", "Cant", cantPenas + 1);
				WriteVar(GetCharPath(UserName), "PENAS", "P" + vb6::CStr(cantPenas + 1),
						vb6::LCase(UserList[UserIndex].Name) + ": UNBAN. " + vb6::CStr(vb6::Now()));

				LogGM(UserList[UserIndex].Name, "/UNBAN a " + UserName);
				WriteConsoleMsg(UserIndex, UserName + " unbanned.", FontTypeNames_FONTTYPE_INFO);
			} else {
				WriteConsoleMsg(UserIndex, UserName + " no está baneado. Imposible unbanear.",
						FontTypeNames_FONTTYPE_INFO);
			}
		}
	}

	/* 'If we got here then packet is complete, copy data back to original queue */


}

/* '' */
/* ' Handles the "NPCFollow" message. */
/* ' */
/* ' @param    userIndex The index of the user sending the message. */

void HandleNPCFollow(int UserIndex) {
	/* '*************************************************** */
	/* 'Author: Nicolas Matias Gonzalez (NIGO) */
	/* 'Last Modification: 12/29/06 */
	/* ' */
	/* '*************************************************** */
	/* 'Remove packet ID */
	UserList[UserIndex].sockctx->incomingData->ReadByte();

	if (UserTieneAlgunPrivilegios(UserIndex, PlayerType_User, PlayerType_Consejero)) {
		return;
	}

	if (UserList[UserIndex].flags.TargetNPC > 0) {
		DoFollow(UserList[UserIndex].flags.TargetNPC, UserList[UserIndex].Name);
		Npclist[UserList[UserIndex].flags.TargetNPC].flags.Inmovilizado = 0;
		Npclist[UserList[UserIndex].flags.TargetNPC].flags.Paralizado = 0;
		Npclist[UserList[UserIndex].flags.TargetNPC].Contadores.Paralisis = 0;
	}
}

/* '' */
/* ' Handles the "SummonChar" message. */
/* ' */
/* ' @param    userIndex The index of the user sending the message. */

void HandleSummonChar(int UserIndex) {
	/* '*************************************************** */
	/* 'Author: Nicolas Matias Gonzalez (NIGO) */
	/* 'Last Modification: 26/03/2009 */
	/* '26/03/2009: ZaMa - Chequeo que no se teletransporte donde haya un char o npc */
	/* '*************************************************** */
	if (UserList[UserIndex].sockctx->incomingData->length() < 3) {
		throw insufficient_data_error();
		return;
	}

	/* 'This packet contains strings, make a copy of the data to prevent losses if it's not complete yet... */
	clsByteQueue* buffer = UserList[UserIndex].sockctx->incomingData.get();


	/* 'Remove packet ID */
	buffer->ReadByte();

	std::string UserName;
	int tUser = 0;
	int X;
	int Y;

	UserName = buffer->ReadUnicodeString();

	if (UserTieneAlgunPrivilegios(UserIndex, PlayerType_Admin, PlayerType_Dios, PlayerType_SemiDios)) {
		tUser = NameIndex(UserName);

		if (tUser <= 0) {
			if (EsDios(UserName) || EsAdmin(UserName)) {
				WriteConsoleMsg(UserIndex, "No puedes invocar a dioses y admins.",
						FontTypeNames_FONTTYPE_INFO);
			} else {
				WriteConsoleMsg(UserIndex, "El jugador no está online.", FontTypeNames_FONTTYPE_INFO);
			}

		} else {
			if (UserTieneAlgunPrivilegios(UserIndex, PlayerType_Dios, PlayerType_Admin)
					|| UserTieneAlgunPrivilegios(tUser, PlayerType_Consejero, PlayerType_User)) {
				WriteConsoleMsg(tUser, UserList[UserIndex].Name + " te ha trasportado.",
						FontTypeNames_FONTTYPE_INFO);
				X = UserList[UserIndex].Pos.X;
				Y = UserList[UserIndex].Pos.Y + 1;
				FindLegalPos(tUser, UserList[UserIndex].Pos.Map, X, Y);
				WarpUserChar(tUser, UserList[UserIndex].Pos.Map, X, Y, true, true);
				LogGM(UserList[UserIndex].Name,
						 vb6::string_format("/SUM %s Map: %d X: %d Y: %d",
								UserName.c_str(),
								UserList[UserIndex].Pos.Map,
								UserList[UserIndex].Pos.X,
								UserList[UserIndex].Pos.Y));
			} else {
				WriteConsoleMsg(UserIndex, "No puedes invocar a dioses y admins.",
						FontTypeNames_FONTTYPE_INFO);
			}
		}
	}

	/* 'If we got here then packet is complete, copy data back to original queue */


}

/* '' */
/* ' Handles the "SpawnListRequest" message. */
/* ' */
/* ' @param    userIndex The index of the user sending the message. */

void HandleSpawnListRequest(int UserIndex) {
	/* '*************************************************** */
	/* 'Author: Nicolas Matias Gonzalez (NIGO) */
	/* 'Last Modification: 12/29/06 */
	/* ' */
	/* '*************************************************** */
	/* 'Remove packet ID */
	UserList[UserIndex].sockctx->incomingData->ReadByte();

	if (UserTieneAlgunPrivilegios(UserIndex, PlayerType_User, PlayerType_Consejero)) {
		return;
	}

	EnviarSpawnList(UserIndex);
}

/* '' */
/* ' Handles the "SpawnCreature" message. */
/* ' */
/* ' @param    userIndex The index of the user sending the message. */

void HandleSpawnCreature(int UserIndex) {
	/* '*************************************************** */
	/* 'Author: Nicolas Matias Gonzalez (NIGO) */
	/* 'Last Modification: 12/29/06 */
	/* ' */
	/* '*************************************************** */
	if (UserList[UserIndex].sockctx->incomingData->length() < 3) {
		throw insufficient_data_error();
		return;
	}

	/* 'Remove packet ID */
	UserList[UserIndex].sockctx->incomingData->ReadByte();

	int npc;
	npc = UserList[UserIndex].sockctx->incomingData->ReadInteger();

	if (UserTieneAlgunPrivilegios(UserIndex, PlayerType_Admin, PlayerType_Dios, PlayerType_SemiDios)) {
		if (npc > 0 && npc <= vb6::UBound(SpawnList)) {
			SpawnNpc(SpawnList[npc].NpcIndex, UserList[UserIndex].Pos, true, false);
		}

		LogGM(UserList[UserIndex].Name, "Sumoneo " + SpawnList[npc].NpcName);
	}
}

/* '' */
/* ' Handles the "ResetNPCInventory" message. */
/* ' */
/* ' @param    userIndex The index of the user sending the message. */

void HandleResetNPCInventory(int UserIndex) {
	/* '*************************************************** */
	/* 'Author: Nicolas Matias Gonzalez (NIGO) */
	/* 'Last Modification: 12/29/06 */
	/* ' */
	/* '*************************************************** */
	/* 'Remove packet ID */
	UserList[UserIndex].sockctx->incomingData->ReadByte();

	if (UserTieneAlgunPrivilegios(UserIndex, PlayerType_User, PlayerType_Consejero, PlayerType_RoleMaster)) {
		return;
	}

	if (UserList[UserIndex].flags.TargetNPC == 0) {
		return;
	}

	ResetNpcInv(UserList[UserIndex].flags.TargetNPC);
	LogGM(UserList[UserIndex].Name, "/RESETINV " + Npclist[UserList[UserIndex].flags.TargetNPC].Name);
}

/* '' */
/* ' Handles the "CleanWorld" message. */
/* ' */
/* ' @param    userIndex The index of the user sending the message. */

void HandleCleanWorld(int UserIndex) {
	/* '*************************************************** */
	/* 'Author: Nicolas Matias Gonzalez (NIGO) */
	/* 'Last Modification: 12/29/06 */
	/* ' */
	/* '*************************************************** */
	/* 'Remove packet ID */
	UserList[UserIndex].sockctx->incomingData->ReadByte();

	if (UserTieneAlgunPrivilegios(UserIndex, PlayerType_User, PlayerType_Consejero, PlayerType_RoleMaster)) {
		return;
	}

	LimpiarMundo();
}

/* '' */
/* ' Handles the "ServerMessage" message. */
/* ' */
/* ' @param    userIndex The index of the user sending the message. */

void HandleServerMessage(int UserIndex) {
	/* '*************************************************** */
	/* 'Author: Nicolas Matias Gonzalez (NIGO) */
	/* 'Last Modification: 28/05/2010 */
	/* '28/05/2010: ZaMa - Ahora no dice el nombre del gm que lo dice. */
	/* '*************************************************** */
	if (UserList[UserIndex].sockctx->incomingData->length() < 3) {
		throw insufficient_data_error();
		return;
	}

	/* 'This packet contains strings, make a copy of the data to prevent losses if it's not complete yet... */
	clsByteQueue* buffer = UserList[UserIndex].sockctx->incomingData.get();


	/* 'Remove packet ID */
	buffer->ReadByte();

	std::string message;
	message = buffer->ReadUnicodeString();

	if (UserTieneAlgunPrivilegios(UserIndex, PlayerType_Admin, PlayerType_Dios, PlayerType_SemiDios)) {
		if (vb6::LenB(message) != 0) {
			LogGM(UserList[UserIndex].Name, "Mensaje Broadcast:" + message);
			SendData(SendTarget_ToAll, 0, PrepareMessageConsoleMsg(message, FontTypeNames_FONTTYPE_TALK));
			/* ''''''''''''''''SOLO PARA EL TESTEO''''''' */
			/* ''''''''''SE USA PARA COMUNICARSE CON EL SERVER''''''''''' */
			/* 'frmMain.txtChat.Text = frmMain.txtChat.Text & vbNewLine & UserList(UserIndex).name & " > " & message */
		}
	}

	/* 'If we got here then packet is complete, copy data back to original queue */


}

/* '' */
/* ' Handles the "MapMessage" message. */
/* ' */
/* ' @param    userIndex The index of the user sending the message. */

void HandleMapMessage(int UserIndex) {
	/* '*************************************************** */
	/* 'Author: ZaMa */
	/* 'Last Modification: 14/11/2010 */
	/* '*************************************************** */
	if (UserList[UserIndex].sockctx->incomingData->length() < 3) {
		throw insufficient_data_error();
		return;
	}

	/* 'This packet contains strings, make a copy of the data to prevent losses if it's not complete yet... */
	clsByteQueue* buffer = UserList[UserIndex].sockctx->incomingData.get();


	/* 'Remove packet ID */
	buffer->ReadByte();

	std::string message;
	message = buffer->ReadUnicodeString();

	if (UserTieneAlgunPrivilegios(UserIndex, PlayerType_Admin, PlayerType_Dios, PlayerType_SemiDios)
			|| (UserTienePrivilegio(UserIndex, PlayerType_Consejero)
					&& UserTienePrivilegio(UserIndex, PlayerType_RoleMaster))) {

		if (vb6::LenB(message) != 0) {

			int mapa;
			mapa = UserList[UserIndex].Pos.Map;

			LogGM(UserList[UserIndex].Name, "Mensaje a mapa " + vb6::CStr(mapa) + ":" + message);
			SendData(SendTarget_toMap, mapa, PrepareMessageConsoleMsg(message, FontTypeNames_FONTTYPE_TALK));
		}
	}

	/* 'If we got here then packet is complete, copy data back to original queue */


}

/* '' */
/* ' Handles the "NickToIP" message. */
/* ' */
/* ' @param    userIndex The index of the user sending the message. */

void HandleNickToIP(int UserIndex) {
	/* '*************************************************** */
	/* 'Author: Nicolas Matias Gonzalez (NIGO) */
	/* 'Last Modification: 07/06/2010 */
	/* 'Pablo (ToxicWaste): Agrego para que el /nick2ip tambien diga los nicks en esa ip por pedido de la DGM. */
	/* '07/06/2010: ZaMa - Ahora no se puede usar para saber si hay dioses/admins online. */
	/* '*************************************************** */
	if (UserList[UserIndex].sockctx->incomingData->length() < 3) {
		throw insufficient_data_error();
		return;
	}

	/* 'This packet contains strings, make a copy of the data to prevent losses if it's not complete yet... */
	clsByteQueue* buffer = UserList[UserIndex].sockctx->incomingData.get();


	/* 'Remove packet ID */
	buffer->ReadByte();

	std::string UserName;
	int tUser = 0;
	bool IsAdmin = false;

	UserName = buffer->ReadUnicodeString();

	if (!UserTienePrivilegio(UserIndex, PlayerType_RoleMaster)
			&& UserTieneAlgunPrivilegios(UserIndex, PlayerType_Admin, PlayerType_Dios, PlayerType_SemiDios)) {
		tUser = NameIndex(UserName);
		LogGM(UserList[UserIndex].Name, "NICK2IP Solicito la IP de " + UserName);

		IsAdmin = UserTieneAlgunPrivilegios(UserIndex, PlayerType_Dios, PlayerType_Admin);

		if (tUser > 0) {
			bool valid = false;

			if (IsAdmin) {
				valid = UserTieneAlgunPrivilegios(tUser, PlayerType_User, PlayerType_Consejero, PlayerType_SemiDios, PlayerType_Dios, PlayerType_Admin);
			} else {
				valid = UserTieneAlgunPrivilegios(tUser, PlayerType_User);
			}

			if (valid) {
				WriteConsoleMsg(UserIndex, "El ip de " + UserName + " es " + UserList[tUser].ip,
						FontTypeNames_FONTTYPE_INFO);
				std::string ip;
				std::string lista;
				int LoopC;
				ip = UserList[tUser].ip;
				for (LoopC = (1); LoopC <= (LastUser); LoopC++) {
					if (UserList[LoopC].ip == ip) {
						if (vb6::LenB(UserList[LoopC].Name) != 0 && UserList[LoopC].flags.UserLogged) {
							if (IsAdmin || UserTienePrivilegio(LoopC, PlayerType_User)) {
								lista = lista + UserList[LoopC].Name + ", ";
							}
						}
					}
				}
				if (vb6::LenB(lista) != 0) {
					lista = vb6::Left(lista, vb6::Len(lista) - 2);
				}
				WriteConsoleMsg(UserIndex, "Los personajes con ip " + ip + " son: " + lista,
						FontTypeNames_FONTTYPE_INFO);
			}
		} else {
			if (!(EsDios(UserName) || EsAdmin(UserName)) || IsAdmin) {
				WriteConsoleMsg(UserIndex, "No hay ningún personaje con ese nick.",
						FontTypeNames_FONTTYPE_INFO);
			}
		}
	}

	/* 'If we got here then packet is complete, copy data back to original queue */


}

/* '' */
/* ' Handles the "IPToNick" message. */
/* ' */
/* ' @param    userIndex The index of the user sending the message. */

void HandleIPToNick(int UserIndex) {
	/* '*************************************************** */
	/* 'Author: Nicolas Matias Gonzalez (NIGO) */
	/* 'Last Modification: 12/29/06 */
	/* ' */
	/* '*************************************************** */
	if (UserList[UserIndex].sockctx->incomingData->length() < 5) {
		throw insufficient_data_error();
		return;
	}

	/* 'Remove packet ID */
	UserList[UserIndex].sockctx->incomingData->ReadByte();

	std::string ip;
	int LoopC;
	std::string lista;

	{
		int a = UserList[UserIndex].sockctx->incomingData->ReadByte();
		int b = UserList[UserIndex].sockctx->incomingData->ReadByte();
		int c = UserList[UserIndex].sockctx->incomingData->ReadByte();
		int d = UserList[UserIndex].sockctx->incomingData->ReadByte();
		ip = vb6::string_format("%d.%d.%d.%d", a, b, c, d);
	}

	if (UserTieneAlgunPrivilegios(UserIndex, PlayerType_User, PlayerType_Consejero, PlayerType_RoleMaster)) {
		return;
	}

	LogGM(UserList[UserIndex].Name, "IP2NICK Solicito los Nicks de IP " + ip);
	bool IsAdmin;

	if (UserTieneAlgunPrivilegios(UserIndex, PlayerType_Dios, PlayerType_Admin)) {
		IsAdmin = true;
	} else {
		IsAdmin = false;
	}

	for (LoopC = (1); LoopC <= (LastUser); LoopC++) {
		if (UserList[LoopC].ip == ip) {
			if (vb6::LenB(UserList[LoopC].Name) != 0 && UserList[LoopC].flags.UserLogged) {
				if (IsAdmin || UserTienePrivilegio(LoopC, PlayerType_User)) {
					lista = lista + UserList[LoopC].Name + ", ";
				}
			}
		}
	}

	if (vb6::LenB(lista) != 0) {
		lista = vb6::Left(lista, vb6::Len(lista) - 2);
	}
	WriteConsoleMsg(UserIndex, "Los personajes con ip " + ip + " son: " + lista, FontTypeNames_FONTTYPE_INFO);
}

/* '' */
/* ' Handles the "GuildOnlineMembers" message. */
/* ' */
/* ' @param    userIndex The index of the user sending the message. */

void HandleGuildOnlineMembers(int UserIndex) {
	/* '*************************************************** */
	/* 'Author: Nicolas Matias Gonzalez (NIGO) */
	/* 'Last Modification: 12/29/06 */
	/* ' */
	/* '*************************************************** */
	if (UserList[UserIndex].sockctx->incomingData->length() < 3) {
		throw insufficient_data_error();
		return;
	}

	/* 'This packet contains strings, make a copy of the data to prevent losses if it's not complete yet... */
	clsByteQueue* buffer = UserList[UserIndex].sockctx->incomingData.get();


	/* 'Remove packet ID */
	buffer->ReadByte();

	std::string GuildName;
	int tGuild;

	GuildName = buffer->ReadUnicodeString();

	if ((vb6::InStrB(GuildName, "+") != 0)) {
		GuildName = vb6::Replace(GuildName, "+", " ");
	}

	if (!UserTieneAlgunPrivilegios(UserIndex, PlayerType_RoleMaster)
			&& UserTieneAlgunPrivilegios(UserIndex, PlayerType_Admin, PlayerType_Dios, PlayerType_SemiDios)) {
		tGuild = GetGuildIndex(GuildName);

		if (tGuild > 0) {
			WriteConsoleMsg(UserIndex,
					"Clan " + vb6::UCase(GuildName) + ": " + m_ListaDeMiembrosOnline(UserIndex, tGuild),
					FontTypeNames_FONTTYPE_GUILDMSG);
		}
	}

	/* 'If we got here then packet is complete, copy data back to original queue */


}

/* '' */
/* ' Handles the "TeleportCreate" message. */
/* ' */
/* ' @param    userIndex The index of the user sending the message. */

void HandleTeleportCreate(int UserIndex) {
	/* '*************************************************** */
	/* 'Author: Nicolas Matias Gonzalez (NIGO) */
	/* 'Last Modification: 22/03/2010 */
	/* '15/11/2009: ZaMa - Ahora se crea un teleport con un radio especificado. */
	/* '22/03/2010: ZaMa - Harcodeo los teleps y radios en el dat, para evitar mapas bugueados. */
	/* '*************************************************** */
	if (UserList[UserIndex].sockctx->incomingData->length() < 6) {
		throw insufficient_data_error();
		return;
	}

	/* 'Remove packet ID */
	UserList[UserIndex].sockctx->incomingData->ReadByte();

	int mapa;
	int X;
	int Y;
	int Radio;

	mapa = UserList[UserIndex].sockctx->incomingData->ReadInteger();
	X = UserList[UserIndex].sockctx->incomingData->ReadByte();
	Y = UserList[UserIndex].sockctx->incomingData->ReadByte();
	Radio = UserList[UserIndex].sockctx->incomingData->ReadByte();

	Radio = MinimoInt(Radio, 6);

	if (UserTieneAlgunPrivilegios(UserIndex, PlayerType_User, PlayerType_Consejero, PlayerType_SemiDios)) {
		return;
	}

	LogGM(UserList[UserIndex].Name, vb6::string_format("/CT %d,%d,%d,%d", mapa, X, Y, Radio));

	if (!MapaValido(mapa) || !InMapBounds(mapa, X, Y)) {
		return;
	}

	if (MapData[UserList[UserIndex].Pos.Map][UserList[UserIndex].Pos.X][UserList[UserIndex].Pos.Y - 1].ObjInfo.ObjIndex
			> 0) {
		return;
	}

	if (MapData[UserList[UserIndex].Pos.Map][UserList[UserIndex].Pos.X][UserList[UserIndex].Pos.Y - 1].TileExit.Map
			> 0) {
		return;
	}

	if (MapData[mapa][X][Y].ObjInfo.ObjIndex > 0) {
		WriteConsoleMsg(UserIndex, "Hay un objeto en el piso en ese lugar.", FontTypeNames_FONTTYPE_INFO);
		return;
	}

	if (MapData[mapa][X][Y].TileExit.Map > 0) {
		WriteConsoleMsg(UserIndex, "No puedes crear un teleport que apunte a la entrada de otro.",
				FontTypeNames_FONTTYPE_INFO);
		return;
	}

	struct Obj ET;
	ET.Amount = 1;
	/* ' Es el numero en el dat. El indice es el comienzo + el radio, todo harcodeado :(. */
	ET.ObjIndex = TELEP_OBJ_INDEX + Radio;

	MapData[UserList[UserIndex].Pos.Map][UserList[UserIndex].Pos.X][UserList[UserIndex].Pos.Y - 1].TileExit.Map =
			mapa;
	MapData[UserList[UserIndex].Pos.Map][UserList[UserIndex].Pos.X][UserList[UserIndex].Pos.Y - 1].TileExit.X =
			X;
	MapData[UserList[UserIndex].Pos.Map][UserList[UserIndex].Pos.X][UserList[UserIndex].Pos.Y - 1].TileExit.Y =
			Y;

	MakeObj(ET, UserList[UserIndex].Pos.Map, UserList[UserIndex].Pos.X, UserList[UserIndex].Pos.Y - 1);
}

/* '' */
/* ' Handles the "TeleportDestroy" message. */
/* ' */
/* ' @param    userIndex The index of the user sending the message. */

void HandleTeleportDestroy(int UserIndex) {
	/* '*************************************************** */
	/* 'Author: Nicolas Matias Gonzalez (NIGO) */
	/* 'Last Modification: 12/29/06 */
	/* ' */
	/* '*************************************************** */
	int mapa;
	int X;
	int Y;

	/* 'Remove packet ID */
	UserList[UserIndex].sockctx->incomingData->ReadByte();

	/* '/dt */
	if (UserTieneAlgunPrivilegios(UserIndex, PlayerType_User, PlayerType_Consejero, PlayerType_SemiDios)) {
		return;
	}

	mapa = UserList[UserIndex].flags.TargetMap;
	X = UserList[UserIndex].flags.TargetX;
	Y = UserList[UserIndex].flags.TargetY;

	if (!InMapBounds(mapa, X, Y)) {
		return;
	}

	if (MapData[mapa][X][Y].ObjInfo.ObjIndex == 0) {
		return;
	}

	if (ObjData[MapData[mapa][X][Y].ObjInfo.ObjIndex].OBJType == eOBJType_otTeleport
			&& MapData[mapa][X][Y].TileExit.Map > 0) {
		LogGM(UserList[UserIndex].Name, vb6::string_format("/DT: %d,%d,%d", mapa, X, Y));

		EraseObj(MapData[mapa][X][Y].ObjInfo.Amount, mapa, X, Y);

		if (MapData[MapData[mapa][X][Y].TileExit.Map][MapData[mapa][X][Y].TileExit.X][MapData[mapa][X][Y].TileExit.Y].ObjInfo.ObjIndex
				== 651) {
			EraseObj(1, MapData[mapa][X][Y].TileExit.Map, MapData[mapa][X][Y].TileExit.X,
					MapData[mapa][X][Y].TileExit.Y);
		}

		MapData[mapa][X][Y].TileExit.Map = 0;
		MapData[mapa][X][Y].TileExit.X = 0;
		MapData[mapa][X][Y].TileExit.Y = 0;
	}
}

/* '' */
/* ' Handles the "RainToggle" message. */
/* ' */
/* ' @param    userIndex The index of the user sending the message. */

void HandleRainToggle(int UserIndex) {
	/* '*************************************************** */
	/* 'Author: Nicolas Matias Gonzalez (NIGO) */
	/* 'Last Modification: 12/29/06 */
	/* ' */
	/* '*************************************************** */
	/* 'Remove packet ID */
	UserList[UserIndex].sockctx->incomingData->ReadByte();

	if (UserTieneAlgunPrivilegios(UserIndex, PlayerType_User, PlayerType_Consejero)) {
		return;
	}

	LogGM(UserList[UserIndex].Name, "/LLUVIA");
	Lloviendo = !Lloviendo;

	SendData(SendTarget_ToAll, 0, PrepareMessageRainToggle());
}

/* '' */
/* ' Handles the "EnableDenounces" message. */
/* ' */
/* ' @param    userIndex The index of the user sending the message. */

void HandleEnableDenounces(int UserIndex) {
	/* '*************************************************** */
	/* 'Author: ZaMa */
	/* 'Last Modification: 14/11/2010 */
	/* 'Enables/Disables */
	/* '*************************************************** */

	/* 'Remove packet ID */
	UserList[UserIndex].sockctx->incomingData->ReadByte();

	/* ' Gm? */
	if (!EsGm(UserIndex)) {
		return;
	}
	/* ' Rm? */
	if (UserTieneAlgunPrivilegios(UserIndex, PlayerType_RoleMaster)) {
		return;
	}

	bool Activado;
	std::string msg;

	Activado = !UserList[UserIndex].flags.SendDenounces;
	UserList[UserIndex].flags.SendDenounces = Activado;

	msg = std::string("Denuncias por consola ") + vb6::IIf(Activado, "ativadas", "desactivadas") + ".";

	LogGM(UserList[UserIndex].Name, msg);

	WriteConsoleMsg(UserIndex, msg, FontTypeNames_FONTTYPE_INFO);

}

/* '' */
/* ' Handles the "ShowDenouncesList" message. */
/* ' */
/* ' @param    userIndex The index of the user sending the message. */

void HandleShowDenouncesList(int UserIndex) {
	/* '*************************************************** */
	/* 'Author: ZaMa */
	/* 'Last Modification: 14/11/2010 */
	/* ' */
	/* '*************************************************** */
	/* 'Remove packet ID */
	UserList[UserIndex].sockctx->incomingData->ReadByte();

	if (UserTieneAlgunPrivilegios(UserIndex, PlayerType_User, PlayerType_RoleMaster)) {
		return;
	}
	WriteShowDenounces(UserIndex);
}

/* '' */
/* ' Handles the "SetCharDescription" message. */
/* ' */
/* ' @param    userIndex The index of the user sending the message. */

void HandleSetCharDescription(int UserIndex) {
	/* '*************************************************** */
	/* 'Author: Nicolas Matias Gonzalez (NIGO) */
	/* 'Last Modification: 12/29/06 */
	/* ' */
	/* '*************************************************** */
	if (UserList[UserIndex].sockctx->incomingData->length() < 3) {
		throw insufficient_data_error();
		return;
	}

	/* 'This packet contains strings, make a copy of the data to prevent losses if it's not complete yet... */
	clsByteQueue* buffer = UserList[UserIndex].sockctx->incomingData.get();


	/* 'Remove packet ID */
	buffer->ReadByte();

	int tUser = 0;
	std::string desc;

	desc = buffer->ReadUnicodeString();

	if (UserTieneAlgunPrivilegios(UserIndex, PlayerType_Dios, PlayerType_Admin, PlayerType_RoleMaster)) {
		tUser = UserList[UserIndex].flags.TargetUser;
		if (tUser > 0) {
			UserList[tUser].DescRM = desc;
		} else {
			WriteConsoleMsg(UserIndex, "Haz click sobre un personaje antes.", FontTypeNames_FONTTYPE_INFO);
		}
	}

	/* 'If we got here then packet is complete, copy data back to original queue */


}

/* '' */
/* ' Handles the "ForceMIDIToMap" message. */
/* ' */
/* ' @param    userIndex The index of the user sending the message. */

void HanldeForceMIDIToMap(int UserIndex) {
	/* '*************************************************** */
	/* 'Author: Nicolas Matias Gonzalez (NIGO) */
	/* 'Last Modification: 12/29/06 */
	/* ' */
	/* '*************************************************** */
	if (UserList[UserIndex].sockctx->incomingData->length() < 4) {
		throw insufficient_data_error();
		return;
	}

	/* 'Remove packet ID */
	UserList[UserIndex].sockctx->incomingData->ReadByte();

	int midiID;
	int mapa;

	midiID = UserList[UserIndex].sockctx->incomingData->ReadByte();
	mapa = UserList[UserIndex].sockctx->incomingData->ReadInteger();

	/* 'Solo dioses, admins y RMS */
	if (UserTieneAlgunPrivilegios(UserIndex, PlayerType_Dios, PlayerType_Admin, PlayerType_RoleMaster)) {
		/* 'Si el mapa no fue enviado tomo el actual */
		if (!InMapBounds(mapa, 50, 50)) {
			mapa = UserList[UserIndex].Pos.Map;
		}

		if (midiID == 0) {
			/* 'Ponemos el default del mapa */
			SendData(SendTarget_toMap, mapa,
					PrepareMessagePlayMidi(MapInfo[UserList[UserIndex].Pos.Map].Music));
		} else {
			/* 'Ponemos el pedido por el GM */
			SendData(SendTarget_toMap, mapa, PrepareMessagePlayMidi(midiID));
		}
	}
}

/* '' */
/* ' Handles the "ForceWAVEToMap" message. */
/* ' */
/* ' @param    userIndex The index of the user sending the message. */

void HandleForceWAVEToMap(int UserIndex) {
	/* '*************************************************** */
	/* 'Author: Nicolas Matias Gonzalez (NIGO) */
	/* 'Last Modification: 12/29/06 */
	/* ' */
	/* '*************************************************** */
	if (UserList[UserIndex].sockctx->incomingData->length() < 6) {
		throw insufficient_data_error();
		return;
	}

	/* 'Remove packet ID */
	UserList[UserIndex].sockctx->incomingData->ReadByte();

	int waveID;
	int mapa;
	int X;
	int Y;

	waveID = UserList[UserIndex].sockctx->incomingData->ReadByte();
	mapa = UserList[UserIndex].sockctx->incomingData->ReadInteger();
	X = UserList[UserIndex].sockctx->incomingData->ReadByte();
	Y = UserList[UserIndex].sockctx->incomingData->ReadByte();

	/* 'Solo dioses, admins y RMS */
	if (UserTieneAlgunPrivilegios(UserIndex, PlayerType_Dios, PlayerType_Admin, PlayerType_RoleMaster)) {
		/* 'Si el mapa no fue enviado tomo el actual */
		if (!InMapBounds(mapa, X, Y)) {
			mapa = UserList[UserIndex].Pos.Map;
			X = UserList[UserIndex].Pos.X;
			Y = UserList[UserIndex].Pos.Y;
		}

		/* 'Ponemos el pedido por el GM */
		SendData(SendTarget_toMap, mapa, PrepareMessagePlayWave(waveID, X, Y));
	}
}

/* '' */
/* ' Handles the "RoyalArmyMessage" message. */
/* ' */
/* ' @param    userIndex The index of the user sending the message. */

void HandleRoyalArmyMessage(int UserIndex) {
	/* '*************************************************** */
	/* 'Author: Nicolas Matias Gonzalez (NIGO) */
	/* 'Last Modification: 12/29/06 */
	/* ' */
	/* '*************************************************** */
	if (UserList[UserIndex].sockctx->incomingData->length() < 3) {
		throw insufficient_data_error();
		return;
	}

	/* 'This packet contains strings, make a copy of the data to prevent losses if it's not complete yet... */
	clsByteQueue* buffer = UserList[UserIndex].sockctx->incomingData.get();


	/* 'Remove packet ID */
	buffer->ReadByte();

	std::string message;
	message = buffer->ReadUnicodeString();

	/* 'Solo dioses, admins, semis y RMS */
	if (UserTieneAlgunPrivilegios(UserIndex, PlayerType_Dios, PlayerType_Admin, PlayerType_SemiDios, PlayerType_RoleMaster)) {
		SendData(SendTarget_ToRealYRMs, 0,
				PrepareMessageConsoleMsg("EJÉRCITO REAL> " + message, FontTypeNames_FONTTYPE_TALK));
	}

	/* 'If we got here then packet is complete, copy data back to original queue */


}

/* '' */
/* ' Handles the "ChaosLegionMessage" message. */
/* ' */
/* ' @param    userIndex The index of the user sending the message. */

void HandleChaosLegionMessage(int UserIndex) {
	/* '*************************************************** */
	/* 'Author: Nicolas Matias Gonzalez (NIGO) */
	/* 'Last Modification: 12/29/06 */
	/* ' */
	/* '*************************************************** */
	if (UserList[UserIndex].sockctx->incomingData->length() < 3) {
		throw insufficient_data_error();
		return;
	}

	/* 'This packet contains strings, make a copy of the data to prevent losses if it's not complete yet... */
	clsByteQueue* buffer = UserList[UserIndex].sockctx->incomingData.get();


	/* 'Remove packet ID */
	buffer->ReadByte();

	std::string message;
	message = buffer->ReadUnicodeString();

	/* 'Solo dioses, admins, semis y RMS */
	if (UserTieneAlgunPrivilegios(UserIndex, PlayerType_Dios, PlayerType_Admin, PlayerType_SemiDios, PlayerType_RoleMaster)) {
		SendData(SendTarget_ToCaosYRMs, 0,
				PrepareMessageConsoleMsg("FUERZAS DEL CAOS> " + message, FontTypeNames_FONTTYPE_TALK));
	}

	/* 'If we got here then packet is complete, copy data back to original queue */


}

/* '' */
/* ' Handles the "CitizenMessage" message. */
/* ' */
/* ' @param    userIndex The index of the user sending the message. */

void HandleCitizenMessage(int UserIndex) {
	/* '*************************************************** */
	/* 'Author: Nicolas Matias Gonzalez (NIGO) */
	/* 'Last Modification: 12/29/06 */
	/* ' */
	/* '*************************************************** */
	if (UserList[UserIndex].sockctx->incomingData->length() < 3) {
		throw insufficient_data_error();
		return;
	}

	/* 'This packet contains strings, make a copy of the data to prevent losses if it's not complete yet... */
	clsByteQueue* buffer = UserList[UserIndex].sockctx->incomingData.get();


	/* 'Remove packet ID */
	buffer->ReadByte();

	std::string message;
	message = buffer->ReadUnicodeString();

	/* 'Solo dioses, admins, semis y RMS */
	if (UserTieneAlgunPrivilegios(UserIndex, PlayerType_Dios, PlayerType_Admin, PlayerType_SemiDios, PlayerType_RoleMaster)) {
		SendData(SendTarget_ToCiudadanosYRMs, 0,
				PrepareMessageConsoleMsg("CIUDADANOS> " + message, FontTypeNames_FONTTYPE_TALK));
	}

	/* 'If we got here then packet is complete, copy data back to original queue */


}

/* '' */
/* ' Handles the "CriminalMessage" message. */
/* ' */
/* ' @param    userIndex The index of the user sending the message. */

void HandleCriminalMessage(int UserIndex) {
	/* '*************************************************** */
	/* 'Author: Nicolas Matias Gonzalez (NIGO) */
	/* 'Last Modification: 12/29/06 */
	/* ' */
	/* '*************************************************** */
	if (UserList[UserIndex].sockctx->incomingData->length() < 3) {
		throw insufficient_data_error();
		return;
	}

	/* 'This packet contains strings, make a copy of the data to prevent losses if it's not complete yet... */
	clsByteQueue* buffer = UserList[UserIndex].sockctx->incomingData.get();


	/* 'Remove packet ID */
	buffer->ReadByte();

	std::string message;
	message = buffer->ReadUnicodeString();

	/* 'Solo dioses, admins y RMS */
	if (UserTieneAlgunPrivilegios(UserIndex, PlayerType_Dios, PlayerType_Admin, PlayerType_RoleMaster)) {
		SendData(SendTarget_ToCriminalesYRMs, 0,
				PrepareMessageConsoleMsg("CRIMINALES> " + message, FontTypeNames_FONTTYPE_TALK));
	}

	/* 'If we got here then packet is complete, copy data back to original queue */


}

/* '' */
/* ' Handles the "TalkAsNPC" message. */
/* ' */
/* ' @param    userIndex The index of the user sending the message. */

void HandleTalkAsNPC(int UserIndex) {
	/* '*************************************************** */
	/* 'Author: Nicolas Matias Gonzalez (NIGO) */
	/* 'Last Modification: 12/29/06 */
	/* ' */
	/* '*************************************************** */
	if (UserList[UserIndex].sockctx->incomingData->length() < 3) {
		throw insufficient_data_error();
		return;
	}

	/* 'This packet contains strings, make a copy of the data to prevent losses if it's not complete yet... */
	clsByteQueue* buffer = UserList[UserIndex].sockctx->incomingData.get();


	/* 'Remove packet ID */
	buffer->ReadByte();

	std::string message;
	message = buffer->ReadUnicodeString();

	/* 'Solo dioses, admins y RMS */
	if (UserTieneAlgunPrivilegios(UserIndex, PlayerType_Dios, PlayerType_Admin, PlayerType_RoleMaster)) {
		/* 'Asegurarse haya un NPC seleccionado */
		if (UserList[UserIndex].flags.TargetNPC > 0) {
			SendData(SendTarget_ToNPCArea, UserList[UserIndex].flags.TargetNPC,
					PrepareMessageChatOverHead(message,
							Npclist[UserList[UserIndex].flags.TargetNPC].Char.CharIndex, 0x00ffffff));
		} else {
			WriteConsoleMsg(UserIndex,
					"Debes seleccionar el NPC por el que quieres hablar antes de usar este comando.",
					FontTypeNames_FONTTYPE_INFO);
		}
	}

	/* 'If we got here then packet is complete, copy data back to original queue */


}

/* '' */
/* ' Handles the "DestroyAllItemsInArea" message. */
/* ' */
/* ' @param    userIndex The index of the user sending the message. */

void HandleDestroyAllItemsInArea(int UserIndex) {
	/* '*************************************************** */
	/* 'Author: Nicolas Matias Gonzalez (NIGO) */
	/* 'Last Modification: 12/30/06 */
	/* ' */
	/* '*************************************************** */
	/* 'Remove packet ID */
	UserList[UserIndex].sockctx->incomingData->ReadByte();

	if (UserTieneAlgunPrivilegios(UserIndex, PlayerType_User, PlayerType_Consejero, PlayerType_SemiDios)) {
		return;
	}

	int X;
	int Y;
	bool bIsExit = false;

	for (Y = (UserList[UserIndex].Pos.Y - MinYBorder + 1); Y <= (UserList[UserIndex].Pos.Y + MinYBorder - 1);
			Y++) {
		for (X = (UserList[UserIndex].Pos.X - MinXBorder + 1);
				X <= (UserList[UserIndex].Pos.X + MinXBorder - 1); X++) {
			if (X > 0 && Y > 0 && X < 101 && Y < 101) {
				if (MapData[UserList[UserIndex].Pos.Map][X][Y].ObjInfo.ObjIndex > 0) {
					bIsExit = MapData[UserList[UserIndex].Pos.Map][X][Y].TileExit.Map > 0;
					if (ItemNoEsDeMapa(MapData[UserList[UserIndex].Pos.Map][X][Y].ObjInfo.ObjIndex,
							bIsExit)) {
						EraseObj(MAX_INVENTORY_OBJS, UserList[UserIndex].Pos.Map, X, Y);
					}
				}
			}
		}
	}

	LogGM(UserList[UserIndex].Name, "/MASSDEST");
}

/* '' */
/* ' Handles the "AcceptRoyalCouncilMember" message. */
/* ' */
/* ' @param    userIndex The index of the user sending the message. */

void HandleAcceptRoyalCouncilMember(int UserIndex) {
	/* '*************************************************** */
	/* 'Author: Nicolas Matias Gonzalez (NIGO) */
	/* 'Last Modification: 12/30/06 */
	/* ' */
	/* '*************************************************** */
	if (UserList[UserIndex].sockctx->incomingData->length() < 3) {
		throw insufficient_data_error();
		return;
	}

	/* 'This packet contains strings, make a copy of the data to prevent losses if it's not complete yet... */
	clsByteQueue* buffer = UserList[UserIndex].sockctx->incomingData.get();


	/* 'Remove packet ID */
	buffer->ReadByte();

	std::string UserName;
	int tUser = 0;

	UserName = buffer->ReadUnicodeString();

	if (!UserTieneAlgunPrivilegios(UserIndex, PlayerType_RoleMaster)
			&& UserTieneAlgunPrivilegios(UserIndex, PlayerType_Admin, PlayerType_Dios)) {
		tUser = NameIndex(UserName);
		if (tUser <= 0) {
			WriteConsoleMsg(UserIndex, "Usuario offline", FontTypeNames_FONTTYPE_INFO);
		} else {
			SendData(SendTarget_ToAll, 0,
					PrepareMessageConsoleMsg(
							UserName + " fue aceptado en el honorable Consejo Real de Banderbill.",
							FontTypeNames_FONTTYPE_CONSEJO));
			if (UserTienePrivilegio(tUser, PlayerType_ChaosCouncil)) {
				UserQuitarPrivilegios(tUser, PlayerType_ChaosCouncil);
			}
			if (!UserTienePrivilegio(tUser, PlayerType_RoyalCouncil)) {
				UserAsignarPrivilegios(tUser, PlayerType_RoyalCouncil);
			}

			WarpUserChar(tUser, UserList[tUser].Pos.Map, UserList[tUser].Pos.X, UserList[tUser].Pos.Y, false);
		}
	}
}

/* '' */
/* ' Handles the "ChaosCouncilMember" message. */
/* ' */
/* ' @param    userIndex The index of the user sending the message. */

void HandleAcceptChaosCouncilMember(int UserIndex) {
	/* '*************************************************** */
	/* 'Author: Nicolas Matias Gonzalez (NIGO) */
	/* 'Last Modification: 12/30/06 */
	/* ' */
	/* '*************************************************** */
	if (UserList[UserIndex].sockctx->incomingData->length() < 3) {
		throw insufficient_data_error();
		return;
	}

	/* 'This packet contains strings, make a copy of the data to prevent losses if it's not complete yet... */
	clsByteQueue* buffer = UserList[UserIndex].sockctx->incomingData.get();


	/* 'Remove packet ID */
	buffer->ReadByte();

	std::string UserName;
	int tUser = 0;

	UserName = buffer->ReadUnicodeString();

	if (!UserTieneAlgunPrivilegios(UserIndex, PlayerType_RoleMaster)
			&& UserTieneAlgunPrivilegios(UserIndex, PlayerType_Admin, PlayerType_Dios)) {
		tUser = NameIndex(UserName);
		if (tUser <= 0) {
			WriteConsoleMsg(UserIndex, "Usuario offline", FontTypeNames_FONTTYPE_INFO);
		} else {
			SendData(SendTarget_ToAll, 0,
					PrepareMessageConsoleMsg(UserName + " fue aceptado en el Concilio de las Sombras.",
							FontTypeNames_FONTTYPE_CONSEJO));

			if (UserTieneAlgunPrivilegios(UserIndex, PlayerType_RoyalCouncil)) {
				UserQuitarPrivilegios(tUser, PlayerType_RoyalCouncil);
			}
			if (!UserTieneAlgunPrivilegios(UserIndex, PlayerType_ChaosCouncil)) {
				UserAsignarPrivilegios(tUser, PlayerType_ChaosCouncil);
			}

			WarpUserChar(tUser, UserList[tUser].Pos.Map, UserList[tUser].Pos.X, UserList[tUser].Pos.Y, false);
		}
	}

	/* 'If we got here then packet is complete, copy data back to original queue */


}

/* '' */
/* ' Handles the "ItemsInTheFloor" message. */
/* ' */
/* ' @param    userIndex The index of the user sending the message. */

void HandleItemsInTheFloor(int UserIndex) {
	/* '*************************************************** */
	/* 'Author: Nicolas Matias Gonzalez (NIGO) */
	/* 'Last Modification: 12/30/06 */
	/* ' */
	/* '*************************************************** */
	/* 'Remove packet ID */
	UserList[UserIndex].sockctx->incomingData->ReadByte();

	if (UserTieneAlgunPrivilegios(UserIndex, PlayerType_User, PlayerType_Consejero, PlayerType_SemiDios)) {
		return;
	}

	int tObj;
	std::string lista;
	int X;
	int Y;

	for (X = (5); X <= (95); X++) {
		for (Y = (5); Y <= (95); Y++) {
			tObj = MapData[UserList[UserIndex].Pos.Map][X][Y].ObjInfo.ObjIndex;
			if (tObj > 0) {
				if (ObjData[tObj].OBJType != eOBJType_otArboles) {
					WriteConsoleMsg(UserIndex, vb6::string_format("(%d, %d)", X, Y) + ObjData[tObj].Name,
							FontTypeNames_FONTTYPE_INFO);
				}
			}
		}
	}
}

/* '' */
/* ' Handles the "MakeDumb" message. */
/* ' */
/* ' @param    userIndex The index of the user sending the message. */

void HandleMakeDumb(int UserIndex) {
	/* '*************************************************** */
	/* 'Author: Nicolas Matias Gonzalez (NIGO) */
	/* 'Last Modification: 12/30/06 */
	/* ' */
	/* '*************************************************** */
	if (UserList[UserIndex].sockctx->incomingData->length() < 3) {
		throw insufficient_data_error();
		return;
	}

	/* 'This packet contains strings, make a copy of the data to prevent losses if it's not complete yet... */
	clsByteQueue* buffer = UserList[UserIndex].sockctx->incomingData.get();


	/* 'Remove packet ID */
	buffer->ReadByte();

	std::string UserName;
	int tUser = 0;

	UserName = buffer->ReadUnicodeString();


	if (UserTieneAlgunPrivilegios(UserIndex, PlayerType_Admin, PlayerType_Dios)
			|| (UserTienePrivilegio(UserIndex, PlayerType_SemiDios)
					&& UserTienePrivilegio(UserIndex, PlayerType_RoleMaster))) {

		tUser = NameIndex(UserName);
		/* 'para deteccion de aoice */
		if (tUser <= 0) {
			WriteConsoleMsg(UserIndex, "Usuario offline.", FontTypeNames_FONTTYPE_INFO);
		} else {
			WriteDumb(tUser);
		}
	}

	/* 'If we got here then packet is complete, copy data back to original queue */


}

/* '' */
/* ' Handles the "MakeDumbNoMore" message. */
/* ' */
/* ' @param    userIndex The index of the user sending the message. */

void HandleMakeDumbNoMore(int UserIndex) {
	/* '*************************************************** */
	/* 'Author: Nicolas Matias Gonzalez (NIGO) */
	/* 'Last Modification: 12/30/06 */
	/* ' */
	/* '*************************************************** */
	if (UserList[UserIndex].sockctx->incomingData->length() < 3) {
		throw insufficient_data_error();
		return;
	}

	/* 'This packet contains strings, make a copy of the data to prevent losses if it's not complete yet... */
	clsByteQueue* buffer = UserList[UserIndex].sockctx->incomingData.get();


	/* 'Remove packet ID */
	buffer->ReadByte();

	std::string UserName;
	int tUser = 0;

	UserName = buffer->ReadUnicodeString();

	if (UserTieneAlgunPrivilegios(UserIndex, PlayerType_Admin, PlayerType_Dios)
			|| (UserTienePrivilegio(UserIndex, PlayerType_SemiDios)
					&& UserTienePrivilegio(UserIndex, PlayerType_RoleMaster))) {
		tUser = NameIndex(UserName);
		/* 'para deteccion de aoice */
		if (tUser <= 0) {
			WriteConsoleMsg(UserIndex, "Usuario offline.", FontTypeNames_FONTTYPE_INFO);
		} else {
			WriteDumbNoMore(tUser);
			FlushBuffer(tUser);
		}
	}

	/* 'If we got here then packet is complete, copy data back to original queue */


}

/* '' */
/* ' Handles the "DumpIPTables" message. */
/* ' */
/* ' @param    userIndex The index of the user sending the message. */

void HandleDumpIPTables(int UserIndex) {
	/* '*************************************************** */
	/* 'Author: Nicolas Matias Gonzalez (NIGO) */
	/* 'Last Modification: 12/30/06 */
	/* ' */
	/* '*************************************************** */
	/* 'Remove packet ID */
	UserList[UserIndex].sockctx->incomingData->ReadByte();

	if (UserTieneAlgunPrivilegios(UserIndex, PlayerType_User, PlayerType_Consejero, PlayerType_SemiDios)) {
		return;
	}

	IpSecurityDumpTables();
}

/* '' */
/* ' Handles the "CouncilKick" message. */
/* ' */
/* ' @param    userIndex The index of the user sending the message. */

void HandleCouncilKick(int UserIndex) {
	/* '*************************************************** */
	/* 'Author: Nicolas Matias Gonzalez (NIGO) */
	/* 'Last Modification: 12/30/06 */
	/* ' */
	/* '*************************************************** */
	if (UserList[UserIndex].sockctx->incomingData->length() < 3) {
		throw insufficient_data_error();
		return;
	}

	/* 'This packet contains strings, make a copy of the data to prevent losses if it's not complete yet... */
	clsByteQueue* buffer = UserList[UserIndex].sockctx->incomingData.get();


	/* 'Remove packet ID */
	buffer->ReadByte();

	std::string UserName;
	int tUser = 0;

	UserName = buffer->ReadUnicodeString();

	if (UserTieneAlgunPrivilegios(UserIndex, PlayerType_Admin, PlayerType_Dios)
			&& (!UserTienePrivilegio(UserIndex, PlayerType_RoleMaster))) {

		tUser = NameIndex(UserName);
		if (tUser <= 0) {
			if (FileExist(GetCharPath(UserName))) {
				WriteConsoleMsg(UserIndex, "Usuario offline, echando de los consejos.",
						FontTypeNames_FONTTYPE_INFO);
				WriteVar(GetCharPath(UserName), "CONSEJO", "PERTENECE", 0);
				WriteVar(GetCharPath(UserName), "CONSEJO", "PERTENECECAOS", 0);
			} else {
				WriteConsoleMsg(UserIndex, "No se encuentra el charfile " + GetCharPath(UserName),
						FontTypeNames_FONTTYPE_INFO);
			}
		} else {
			if (UserTieneAlgunPrivilegios(tUser, PlayerType_RoyalCouncil)) {
				WriteConsoleMsg(tUser, "Has sido echado del consejo de Banderbill.",
						FontTypeNames_FONTTYPE_TALK);
				UserQuitarPrivilegios(tUser, PlayerType_RoyalCouncil);

				WarpUserChar(tUser, UserList[tUser].Pos.Map, UserList[tUser].Pos.X, UserList[tUser].Pos.Y,
						false);
				SendData(SendTarget_ToAll, 0,
						PrepareMessageConsoleMsg(UserName + " fue expulsado del consejo de Banderbill.",
								FontTypeNames_FONTTYPE_CONSEJO));
			}

			if (UserTieneAlgunPrivilegios(tUser, PlayerType_ChaosCouncil)) {
				WriteConsoleMsg(tUser, "Has sido echado del Concilio de las Sombras.",
						FontTypeNames_FONTTYPE_TALK);
				UserQuitarPrivilegios(tUser, PlayerType_ChaosCouncil);

				WarpUserChar(tUser, UserList[tUser].Pos.Map, UserList[tUser].Pos.X, UserList[tUser].Pos.Y,
						false);
				SendData(SendTarget_ToAll, 0,
						PrepareMessageConsoleMsg(UserName + " fue expulsado del Concilio de las Sombras.",
								FontTypeNames_FONTTYPE_CONSEJO));
			}
		}
	}

	/* 'If we got here then packet is complete, copy data back to original queue */


}

/* '' */
/* ' Handles the "SetTrigger" message. */
/* ' */
/* ' @param    userIndex The index of the user sending the message. */

void HandleSetTrigger(int UserIndex) {
	/* '*************************************************** */
	/* 'Author: Nicolas Matias Gonzalez (NIGO) */
	/* 'Last Modification: 12/30/06 */
	/* ' */
	/* '*************************************************** */
	if (UserList[UserIndex].sockctx->incomingData->length() < 2) {
		throw insufficient_data_error();
		return;
	}

	/* 'Remove packet ID */
	UserList[UserIndex].sockctx->incomingData->ReadByte();

	int tTrigger;
	std::string tLog;

	tTrigger = UserList[UserIndex].sockctx->incomingData->ReadByte();

	if (UserTieneAlgunPrivilegios(UserIndex, PlayerType_User, PlayerType_Consejero, PlayerType_SemiDios, PlayerType_RoleMaster)) {
		return;
	}

	if (tTrigger >= 0) {
		MapData[UserList[UserIndex].Pos.Map][UserList[UserIndex].Pos.X][UserList[UserIndex].Pos.Y].trigger =
				static_cast<eTrigger>(tTrigger);
		tLog = vb6::string_format("Trigger %d en mapa %d %d %d",
				tTrigger,
				UserList[UserIndex].Pos.Map,
				UserList[UserIndex].Pos.X,
				UserList[UserIndex].Pos.Y);
		LogGM(UserList[UserIndex].Name, tLog);
		WriteConsoleMsg(UserIndex, tLog, FontTypeNames_FONTTYPE_INFO);
	}
}

/* '' */
/* ' Handles the "AskTrigger" message. */
/* ' */
/* ' @param    userIndex The index of the user sending the message. */

void HandleAskTrigger(int UserIndex) {
	/* '*************************************************** */
	/* 'Author: Nicolas Matias Gonzalez (NIGO) */
	/* 'Last Modification: 04/13/07 */
	/* ' */
	/* '*************************************************** */
	int tTrigger;

	/* 'Remove packet ID */
	UserList[UserIndex].sockctx->incomingData->ReadByte();

	if (UserTieneAlgunPrivilegios(UserIndex, PlayerType_User, PlayerType_Consejero, PlayerType_SemiDios, PlayerType_RoleMaster)) {
		return;
	}

	tTrigger =
			MapData[UserList[UserIndex].Pos.Map][UserList[UserIndex].Pos.X][UserList[UserIndex].Pos.Y].trigger;

	LogGM(UserList[UserIndex].Name,
			vb6::string_format("Miro el trigger %d en mapa %d %d %d",
							tTrigger,
							UserList[UserIndex].Pos.Map,
							UserList[UserIndex].Pos.X,
							UserList[UserIndex].Pos.Y));

	WriteConsoleMsg(UserIndex,
			vb6::string_format("Trigger %d en mapa %d %d %d",
							tTrigger,
							UserList[UserIndex].Pos.Map,
							UserList[UserIndex].Pos.X,
							UserList[UserIndex].Pos.Y),
			FontTypeNames_FONTTYPE_INFO);
}

/* '' */
/* ' Handles the "BannedIPList" message. */
/* ' */
/* ' @param    userIndex The index of the user sending the message. */

void HandleBannedIPList(int UserIndex) {
	/* '*************************************************** */
	/* 'Author: Nicolas Matias Gonzalez (NIGO) */
	/* 'Last Modification: 12/30/06 */
	/* ' */
	/* '*************************************************** */
	/* 'Remove packet ID */
	UserList[UserIndex].sockctx->incomingData->ReadByte();

	if (UserTieneAlgunPrivilegios(UserIndex, PlayerType_User, PlayerType_Consejero, PlayerType_SemiDios, PlayerType_RoleMaster)) {
		return;
	}

	std::string lista;

	LogGM(UserList[UserIndex].Name, "/BANIPLIST");

	for (auto & e : BanIps) {
		lista += e + ", ";
	}

	if (vb6::LenB(lista) != 0) {
		lista = vb6::Left(lista, vb6::Len(lista) - 2);
	}

	WriteConsoleMsg(UserIndex, lista, FontTypeNames_FONTTYPE_INFO);
}

/* '' */
/* ' Handles the "BannedIPReload" message. */
/* ' */
/* ' @param    userIndex The index of the user sending the message. */

void HandleBannedIPReload(int UserIndex) {
	/* '*************************************************** */
	/* 'Author: Nicolas Matias Gonzalez (NIGO) */
	/* 'Last Modification: 12/30/06 */
	/* ' */
	/* '*************************************************** */
	/* 'Remove packet ID */
	UserList[UserIndex].sockctx->incomingData->ReadByte();

	if (UserTieneAlgunPrivilegios(UserIndex, PlayerType_User, PlayerType_Consejero, PlayerType_SemiDios, PlayerType_RoleMaster)) {
		return;
	}

	BanIpGuardar();
	BanIpCargar();
}

/* '' */
/* ' Handles the "GuildBan" message. */
/* ' */
/* ' @param    userIndex The index of the user sending the message. */

void HandleGuildBan(int UserIndex) {
	/* '*************************************************** */
	/* 'Author: Nicolas Matias Gonzalez (NIGO) */
	/* 'Last Modification: 12/30/06 */
	/* ' */
	/* '*************************************************** */
	if (UserList[UserIndex].sockctx->incomingData->length() < 3) {
		throw insufficient_data_error();
		return;
	}

	/* 'This packet contains strings, make a copy of the data to prevent losses if it's not complete yet... */
	clsByteQueue* buffer = UserList[UserIndex].sockctx->incomingData.get();


	/* 'Remove packet ID */
	buffer->ReadByte();

	std::string GuildName;
	int cantMembers;
	int LoopC;
	std::string member;
	int Count;
	int tIndex;
	std::string tFile;

	GuildName = buffer->ReadUnicodeString();

	if (UserTieneAlgunPrivilegios(UserIndex, PlayerType_Admin, PlayerType_Dios)
			&& (!UserTienePrivilegio(UserIndex, PlayerType_RoleMaster))) {

		tFile = GetGuildsPath(GuildName, EGUILDPATH::Members);

		if (!FileExist(tFile)) {
			WriteConsoleMsg(UserIndex, "No existe el clan: " + GuildName, FontTypeNames_FONTTYPE_INFO);
		} else {
			SendData(SendTarget_ToAll, 0,
					PrepareMessageConsoleMsg(
							UserList[UserIndex].Name + " baneó al clan " + vb6::UCase(GuildName),
							FontTypeNames_FONTTYPE_FIGHT));

			/* 'baneamos a los miembros */
			LogGM(UserList[UserIndex].Name, "BANCLAN a " + vb6::UCase(GuildName));

			cantMembers = vb6::val(GetVar(tFile, "INIT", "NroMembers"));

			for (LoopC = (1); LoopC <= (cantMembers); LoopC++) {
				member = GetVar(tFile, "Members", "Member" + vb6::CStr(LoopC));
				/* 'member es la victima */
				Ban(member, "Administracion del servidor", "Clan Banned");

				SendData(SendTarget_ToAll, 0,
						PrepareMessageConsoleMsg(
								"   " + member + "<" + GuildName + "> ha sido expulsado del servidor.",
								FontTypeNames_FONTTYPE_FIGHT));

				tIndex = NameIndex(member);
				if (tIndex > 0) {
					/* 'esta online */
					UserList[tIndex].flags.Ban = 1;
					CloseSocket(tIndex);
				}

				/* 'ponemos el flag de ban a 1 */
				WriteVar(GetCharPath(member), "FLAGS", "Ban", "1");
				/* 'ponemos la pena */
				Count = vb6::val(GetVar(GetCharPath(member), "PENAS", "Cant"));
				WriteVar(GetCharPath(member), "PENAS", "Cant", Count + 1);
				WriteVar(GetCharPath(member), "PENAS", "P" + vb6::CStr(Count + 1),
						vb6::LCase(UserList[UserIndex].Name) + ": BAN AL CLAN: " + GuildName + " "
								+ vb6::CStr(vb6::Now()));
			}
		}
	}

	/* 'If we got here then packet is complete, copy data back to original queue */


}

/* '' */
/* ' Handles the "BanIP" message. */
/* ' */
/* ' @param    userIndex The index of the user sending the message. */

void HandleBanIP(int UserIndex) {
	/* '*************************************************** */
	/* 'Author: Juan Martín Sotuyo Dodero (Maraxus) */
	/* 'Last Modification: 07/02/09 */
	/* 'Agregado un CopyBuffer porque se producia un bucle */
	/* 'inifito al intentar banear una ip ya baneada. (NicoNZ) */
	/* '07/02/09 Pato - Ahora no es posible saber si un gm está o no online. */
	/* '*************************************************** */
	if (UserList[UserIndex].sockctx->incomingData->length() < 6) {
		throw insufficient_data_error();
		return;
	}

	/* 'This packet contains strings, make a copy of the data to prevent losses if it's not complete yet... */
	clsByteQueue* buffer = UserList[UserIndex].sockctx->incomingData.get();


	/* 'Remove packet ID */
	buffer->ReadByte();

	std::string bannedIP;
	int tUser = 0;
	std::string Reason;
	int i;

	/* ' Is it by ip?? */
	if (buffer->ReadBoolean()) {
		int a = buffer->ReadByte();
		int b = buffer->ReadByte();
		int c = buffer->ReadByte();
		int d = buffer->ReadByte();
		bannedIP = vb6::string_format("%d.%d.%d.%d", a, b, c, d);
	} else {
		tUser = NameIndex(buffer->ReadUnicodeString());

		if (tUser > 0) {
			bannedIP = UserList[tUser].ip;
		}
	}

	Reason = buffer->ReadUnicodeString();

	if (UserTieneAlgunPrivilegios(UserIndex, PlayerType_Admin, PlayerType_Dios)) {
		if (vb6::LenB(bannedIP) > 0) {
			LogGM(UserList[UserIndex].Name, "/BanIP " + bannedIP + " por " + Reason);

			if (BanIpBuscar(bannedIP) > 0) {
				WriteConsoleMsg(UserIndex, "La IP " + bannedIP + " ya se encuentra en la lista de bans.",
						FontTypeNames_FONTTYPE_INFO);
			} else {
				BanIpAgrega(bannedIP);
				SendData(SendTarget_ToAdmins, 0,
						PrepareMessageConsoleMsg(
								UserList[UserIndex].Name + " baneó la IP " + bannedIP + " por " + Reason,
								FontTypeNames_FONTTYPE_FIGHT));

				/* 'Find every player with that ip and ban him! */
				for (i = (1); i <= (LastUser); i++) {
					if (UserList[i].ConnIDValida) {
						if (UserList[i].ip == bannedIP) {
							BanCharacter(UserIndex, UserList[i].Name, "IP POR " + Reason);
						}
					}
				}
			}
		} else if (tUser <= 0) {
			WriteConsoleMsg(UserIndex, "El personaje no está online.", FontTypeNames_FONTTYPE_INFO);
		}
	}

	/* 'If we got here then packet is complete, copy data back to original queue */


}

/* '' */
/* ' Handles the "UnbanIP" message. */
/* ' */
/* ' @param    userIndex The index of the user sending the message. */

void HandleUnbanIP(int UserIndex) {
	/* '*************************************************** */
	/* 'Author: Juan Martín Sotuyo Dodero (Maraxus) */
	/* 'Last Modification: 12/30/06 */
	/* ' */
	/* '*************************************************** */
	if (UserList[UserIndex].sockctx->incomingData->length() < 5) {
		throw insufficient_data_error();
		return;
	}

	/* 'Remove packet ID */
	UserList[UserIndex].sockctx->incomingData->ReadByte();

	std::string bannedIP;

	{
		int a = UserList[UserIndex].sockctx->incomingData->ReadByte();
		int b = UserList[UserIndex].sockctx->incomingData->ReadByte();
		int c = UserList[UserIndex].sockctx->incomingData->ReadByte();
		int d = UserList[UserIndex].sockctx->incomingData->ReadByte();
		bannedIP = vb6::string_format("%d.%d.%d.%d", a, b, c, d);
	}

	if (UserTieneAlgunPrivilegios(UserIndex, PlayerType_User, PlayerType_Consejero, PlayerType_SemiDios, PlayerType_RoleMaster)) {
		return;
	}

	if (BanIpQuita(bannedIP)) {
		WriteConsoleMsg(UserIndex, "La IP " "" + bannedIP + "" " se ha quitado de la lista de bans.",
				FontTypeNames_FONTTYPE_INFO);
	} else {
		WriteConsoleMsg(UserIndex, "La IP " "" + bannedIP + "" " NO se encuentra en la lista de bans.",
				FontTypeNames_FONTTYPE_INFO);
	}
}

/* '' */
/* ' Handles the "CreateItem" message. */
/* ' */
/* ' @param    userIndex The index of the user sending the message. */

void HandleCreateItem(int UserIndex) {
	/* '*************************************************** */
	/* 'Author: Nicolas Matias Gonzalez (NIGO) */
	/* 'Last Modification: 12/30/06 */
	/* ' */
	/* '*************************************************** */
	if (UserList[UserIndex].sockctx->incomingData->length() < 3) {
		throw insufficient_data_error();
		return;
	}

	/* 'Remove packet ID */
	UserList[UserIndex].sockctx->incomingData->ReadByte();

	int tObj;
	std::string tStr;
	tObj = UserList[UserIndex].sockctx->incomingData->ReadInteger();

	if (UserTieneAlgunPrivilegios(UserIndex, PlayerType_User, PlayerType_Consejero, PlayerType_SemiDios)) {
		return;
	}

	int mapa;
	int X;
	int Y;

	mapa = UserList[UserIndex].Pos.Map;
	X = UserList[UserIndex].Pos.X;
	Y = UserList[UserIndex].Pos.Y;

	LogGM(UserList[UserIndex].Name, vb6::string_format("/CI: %d en mapa %d (%d, %d)", tObj, mapa, X, Y));

	if (MapData[mapa][X][Y - 1].ObjInfo.ObjIndex > 0) {
		return;
	}

	if (MapData[mapa][X][Y - 1].TileExit.Map > 0) {
		return;
	}

	if (tObj < 1 || tObj > NumObjDatas) {
		return;
	}

	/* 'Is the object not null? */
	if (vb6::LenB(ObjData[tObj].Name) == 0) {
		return;
	}

	struct Obj Objeto;
	WriteConsoleMsg(UserIndex,
			"¡¡ATENCIÓN: FUERON CREADOS ***100*** ÍTEMS, TIRE Y /DEST LOS QUE NO NECESITE!!",
			FontTypeNames_FONTTYPE_GUILD);

	Objeto.Amount = 100;
	Objeto.ObjIndex = tObj;
	MakeObj(Objeto, mapa, X, Y - 1);

	if (ObjData[tObj].Log == 1) {
		LogDesarrollo(
				UserList[UserIndex].Name
						+ vb6::string_format(" /CI: [%d] %s en mapa %d (%d, %d)",
								tObj, ObjData[tObj].Name.c_str(), mapa, X, Y));
	}
}

/* '' */
/* ' Handles the "DestroyItems" message. */
/* ' */
/* ' @param    userIndex The index of the user sending the message. */

void HandleDestroyItems(int UserIndex) {
	/* '*************************************************** */
	/* 'Author: Nicolas Matias Gonzalez (NIGO) */
	/* 'Last Modification: 12/30/06 */
	/* ' */
	/* '*************************************************** */
	/* 'Remove packet ID */
	UserList[UserIndex].sockctx->incomingData->ReadByte();

	if (UserTieneAlgunPrivilegios(UserIndex, PlayerType_User, PlayerType_Consejero, PlayerType_SemiDios)) {
		return;
	}

	int mapa;
	int X;
	int Y;

	mapa = UserList[UserIndex].Pos.Map;
	X = UserList[UserIndex].Pos.X;
	Y = UserList[UserIndex].Pos.Y;

	int ObjIndex;
	ObjIndex = MapData[mapa][X][Y].ObjInfo.ObjIndex;

	if (ObjIndex == 0) {
		return;
	}

	LogGM(UserList[UserIndex].Name,
			vb6::string_format("/DEST %d en mapa %d (%d, %d). Cantidad: %d", ObjIndex, mapa, X, Y,
					MapData[mapa][X][Y].ObjInfo.Amount));

	if (ObjData[ObjIndex].OBJType == eOBJType_otTeleport && MapData[mapa][X][Y].TileExit.Map > 0) {

		WriteConsoleMsg(UserIndex, "No puede destruir teleports así. Utilice /DT.",
				FontTypeNames_FONTTYPE_INFO);
		return;
	}

	EraseObj(10000, mapa, X, Y);
}

/* '' */
/* ' Handles the "ChaosLegionKick" message. */
/* ' */
/* ' @param    userIndex The index of the user sending the message. */

void HandleChaosLegionKick(int UserIndex) {
	/* '*************************************************** */
	/* 'Author: Nicolas Matias Gonzalez (NIGO) */
	/* 'Last Modification: 12/30/06 */
	/* ' */
	/* '*************************************************** */
	if (UserList[UserIndex].sockctx->incomingData->length() < 3) {
		throw insufficient_data_error();
		return;
	}

	/* 'This packet contains strings, make a copy of the data to prevent losses if it's not complete yet... */
	clsByteQueue* buffer = UserList[UserIndex].sockctx->incomingData.get();


	/* 'Remove packet ID */
	buffer->ReadByte();

	std::string UserName;
	std::string Reason;
	int tUser = 0;
	int cantPenas;

	UserName = buffer->ReadUnicodeString();
	Reason = buffer->ReadUnicodeString();

	if (UserTieneAlgunPrivilegios(UserIndex, PlayerType_Admin, PlayerType_Dios)
			|| UserList[UserIndex].flags.PrivEspecial) {

		if ((vb6::InStrB(UserName, "/") != 0)) {
			UserName = vb6::Replace(UserName, "/", "");
		}
		if ((vb6::InStrB(UserName, "/") != 0)) {
			UserName = vb6::Replace(UserName, "/", "");
		}
		tUser = NameIndex(UserName);

		LogGM(UserList[UserIndex].Name, "ECHO DEL CAOS A: " + UserName);

		if (tUser > 0) {
			ExpulsarFaccionCaos(tUser, true);
			UserList[tUser].Faccion.Reenlistadas = 200;
			WriteConsoleMsg(UserIndex,
					UserName + " expulsado de las fuerzas del caos y prohibida la reenlistada.",
					FontTypeNames_FONTTYPE_INFO);
			WriteConsoleMsg(tUser,
					UserList[UserIndex].Name
							+ " te ha expulsado en forma definitiva de las fuerzas del caos.",
					FontTypeNames_FONTTYPE_FIGHT);
			FlushBuffer(tUser);

			cantPenas = vb6::val(GetVar(GetCharPath(UserName), "PENAS", "Cant"));
			WriteVar(GetCharPath(UserName), "PENAS", "Cant", cantPenas + 1);
			WriteVar(GetCharPath(UserName), "PENAS", "P" + vb6::CStr(cantPenas + 1),
					vb6::LCase(UserList[UserIndex].Name) + ": EXPULSADO de la Legión Oscura por: "
							+ vb6::LCase(Reason) + " " + vb6::CStr(vb6::Now()));
		} else {
			if (FileExist(GetCharPath(UserName))) {
				WriteVar(GetCharPath(UserName), "FACCIONES", "EjercitoCaos", 0);
				WriteVar(GetCharPath(UserName), "FACCIONES", "Reenlistadas", 200);
				WriteVar(GetCharPath(UserName), "FACCIONES", "Extra",
						"Expulsado por " + UserList[UserIndex].Name);

				cantPenas = vb6::val(GetVar(GetCharPath(UserName), "PENAS", "Cant"));
				WriteVar(GetCharPath(UserName), "PENAS", "Cant", cantPenas + 1);
				WriteVar(GetCharPath(UserName), "PENAS", "P" + vb6::CStr(cantPenas + 1),
						vb6::LCase(UserList[UserIndex].Name) + ": EXPULSADO de la Legión Oscura por: "
								+ vb6::LCase(Reason) + " " + vb6::CStr(vb6::Now()));

				WriteConsoleMsg(UserIndex,
						UserName + " expulsado de las fuerzas del caos y prohibida la reenlistada.",
						FontTypeNames_FONTTYPE_INFO);
			} else {
				WriteConsoleMsg(UserIndex, UserName + ".chr inexistente.", FontTypeNames_FONTTYPE_INFO);
			}
		}
	}

	/* 'If we got here then packet is complete, copy data back to original queue */


}

/* '' */
/* ' Handles the "RoyalArmyKick" message. */
/* ' */
/* ' @param    userIndex The index of the user sending the message. */

void HandleRoyalArmyKick(int UserIndex) {
	/* '*************************************************** */
	/* 'Author: Nicolas Matias Gonzalez (NIGO) */
	/* 'Last Modification: 12/30/06 */
	/* ' */
	/* '*************************************************** */
	if (UserList[UserIndex].sockctx->incomingData->length() < 3) {
		throw insufficient_data_error();
		return;
	}

	/* 'This packet contains strings, make a copy of the data to prevent losses if it's not complete yet... */
	clsByteQueue* buffer = UserList[UserIndex].sockctx->incomingData.get();


	/* 'Remove packet ID */
	buffer->ReadByte();

	std::string UserName;
	std::string Reason;
	int tUser = 0;
	int cantPenas;

	UserName = buffer->ReadUnicodeString();
	Reason = buffer->ReadUnicodeString();

	if (UserTieneAlgunPrivilegios(UserIndex, PlayerType_Admin, PlayerType_Dios)
			|| UserList[UserIndex].flags.PrivEspecial) {

		if ((vb6::InStrB(UserName, "/") != 0)) {
			UserName = vb6::Replace(UserName, "/", "");
		}
		if ((vb6::InStrB(UserName, "/") != 0)) {
			UserName = vb6::Replace(UserName, "/", "");
		}
		tUser = NameIndex(UserName);

		LogGM(UserList[UserIndex].Name, "ECHÓ DE LA REAL A: " + UserName);

		if (tUser > 0) {
			ExpulsarFaccionReal(tUser, true);
			UserList[tUser].Faccion.Reenlistadas = 200;
			WriteConsoleMsg(UserIndex,
					UserName + " expulsado de las fuerzas reales y prohibida la reenlistada.",
					FontTypeNames_FONTTYPE_INFO);
			WriteConsoleMsg(tUser,
					UserList[UserIndex].Name + " te ha expulsado en forma definitiva de las fuerzas reales.",
					FontTypeNames_FONTTYPE_FIGHT);
			FlushBuffer(tUser);

			cantPenas = vb6::val(GetVar(GetCharPath(UserName), "PENAS", "Cant"));
			WriteVar(GetCharPath(UserName), "PENAS", "Cant", cantPenas + 1);
			WriteVar(GetCharPath(UserName), "PENAS", "P" + vb6::CStr(cantPenas + 1),
					vb6::LCase(UserList[UserIndex].Name) + ": EXPULSADO de la Legión Oscura por: "
							+ vb6::LCase(Reason) + " " + vb6::CStr(vb6::Now()));
		} else {
			if (FileExist(GetCharPath(UserName))) {
				WriteVar(GetCharPath(UserName), "FACCIONES", "EjercitoReal", 0);
				WriteVar(GetCharPath(UserName), "FACCIONES", "Reenlistadas", 200);
				WriteVar(GetCharPath(UserName), "FACCIONES", "Extra",
						"Expulsado por " + UserList[UserIndex].Name);
				WriteConsoleMsg(UserIndex,
						UserName + " expulsado de las fuerzas reales y prohibida la reenlistada.",
						FontTypeNames_FONTTYPE_INFO);

				cantPenas = vb6::val(GetVar(GetCharPath(UserName), "PENAS", "Cant"));
				WriteVar(GetCharPath(UserName), "PENAS", "Cant", cantPenas + 1);
				WriteVar(GetCharPath(UserName), "PENAS", "P" + vb6::CStr(cantPenas + 1),
						vb6::LCase(UserList[UserIndex].Name) + ": EXPULSADO de la Legión Oscura por: "
								+ vb6::LCase(Reason) + " " + vb6::CStr(vb6::Now()));
			} else {
				WriteConsoleMsg(UserIndex, UserName + ".chr inexistente.", FontTypeNames_FONTTYPE_INFO);
			}
		}
	}

	/* 'If we got here then packet is complete, copy data back to original queue */


}

/* '' */
/* ' Handles the "ForceMIDIAll" message. */
/* ' */
/* ' @param    userIndex The index of the user sending the message. */

void HandleForceMIDIAll(int UserIndex) {
	/* '*************************************************** */
	/* 'Author: Nicolas Matias Gonzalez (NIGO) */
	/* 'Last Modification: 12/30/06 */
	/* ' */
	/* '*************************************************** */
	if (UserList[UserIndex].sockctx->incomingData->length() < 2) {
		throw insufficient_data_error();
		return;
	}

	/* 'Remove packet ID */
	UserList[UserIndex].sockctx->incomingData->ReadByte();

	int midiID;
	midiID = UserList[UserIndex].sockctx->incomingData->ReadByte();

	if (UserTieneAlgunPrivilegios(UserIndex, PlayerType_User, PlayerType_Consejero, PlayerType_SemiDios)) {
		return;
	}

	SendData(SendTarget_ToAll, 0,
			PrepareMessageConsoleMsg(UserList[UserIndex].Name + " broadcast música: " + vb6::CStr(midiID),
					FontTypeNames_FONTTYPE_SERVER));

	SendData(SendTarget_ToAll, 0, PrepareMessagePlayMidi(midiID));
}

/* '' */
/* ' Handles the "ForceWAVEAll" message. */
/* ' */
/* ' @param    userIndex The index of the user sending the message. */

void HandleForceWAVEAll(int UserIndex) {
	/* '*************************************************** */
	/* 'Author: Nicolas Matias Gonzalez (NIGO) */
	/* 'Last Modification: 12/30/06 */
	/* ' */
	/* '*************************************************** */
	if (UserList[UserIndex].sockctx->incomingData->length() < 2) {
		throw insufficient_data_error();
		return;
	}

	/* 'Remove packet ID */
	UserList[UserIndex].sockctx->incomingData->ReadByte();

	int waveID;
	waveID = UserList[UserIndex].sockctx->incomingData->ReadByte();

	if (UserTieneAlgunPrivilegios(UserIndex, PlayerType_User, PlayerType_Consejero, PlayerType_SemiDios)) {
		return;
	}

	SendData(SendTarget_ToAll, 0, PrepareMessagePlayWave(waveID, NO_3D_SOUND, NO_3D_SOUND));
}

/* '' */
/* ' Handles the "RemovePunishment" message. */
/* ' */
/* ' @param    userIndex The index of the user sending the message. */

void HandleRemovePunishment(int UserIndex) {
	/* '*************************************************** */
	/* 'Author: Nicolas Matias Gonzalez (NIGO) */
	/* 'Last Modification: 1/05/07 */
	/* 'Pablo (ToxicWaste): 1/05/07, You can now edit the punishment. */
	/* '*************************************************** */
	if (UserList[UserIndex].sockctx->incomingData->length() < 6) {
		throw insufficient_data_error();
		return;
	}

	/* 'This packet contains strings, make a copy of the data to prevent losses if it's not complete yet... */
	clsByteQueue* buffer = UserList[UserIndex].sockctx->incomingData.get();


	/* 'Remove packet ID */
	buffer->ReadByte();

	std::string UserName;
	int punishment;
	std::string NewText;

	UserName = buffer->ReadUnicodeString();
	punishment = buffer->ReadByte();
	NewText = buffer->ReadUnicodeString();

	if (UserTieneAlgunPrivilegios(UserIndex, PlayerType_Admin, PlayerType_Dios)
			&& (!UserTienePrivilegio(UserIndex, PlayerType_RoleMaster))) {

		if (vb6::LenB(UserName) == 0) {
			WriteConsoleMsg(UserIndex, "Utilice /borrarpena Nick@NumeroDePena@NuevaPena",
					FontTypeNames_FONTTYPE_INFO);
		} else {
			if ((vb6::InStrB(UserName, "/") != 0)) {
				UserName = vb6::Replace(UserName, "/", "");
			}
			if ((vb6::InStrB(UserName, "/") != 0)) {
				UserName = vb6::Replace(UserName, "/", "");
			}

			if (FileExist(GetCharPath(UserName), 0)) {
				LogGM(UserList[UserIndex].Name,
						" borro la pena: " + vb6::CStr(punishment) + "-"
								+ GetVar(GetCharPath(UserName), "PENAS", "P" + vb6::CStr(punishment)) + " de "
								+ UserName + " y la cambió por: " + NewText);

				WriteVar(GetCharPath(UserName), "PENAS", "P" + vb6::CStr(punishment),
						vb6::LCase(UserList[UserIndex].Name) + ": <" + NewText + "> "
								+ vb6::CStr(vb6::Now()));

				WriteConsoleMsg(UserIndex, "Pena modificada.", FontTypeNames_FONTTYPE_INFO);
			}
		}
	}

	/* 'If we got here then packet is complete, copy data back to original queue */


}

/* '' */
/* ' Handles the "TileBlockedToggle" message. */
/* ' */
/* ' @param    userIndex The index of the user sending the message. */

void HandleTileBlockedToggle(int UserIndex) {
	/* '*************************************************** */
	/* 'Author: Nicolas Matias Gonzalez (NIGO) */
	/* 'Last Modification: 12/30/06 */
	/* ' */
	/* '*************************************************** */
	/* 'Remove packet ID */
	UserList[UserIndex].sockctx->incomingData->ReadByte();

	if (UserTieneAlgunPrivilegios(UserIndex, PlayerType_User, PlayerType_Consejero, PlayerType_SemiDios)) {
		return;
	}

	LogGM(UserList[UserIndex].Name, "/BLOQ");

	if (MapData[UserList[UserIndex].Pos.Map][UserList[UserIndex].Pos.X][UserList[UserIndex].Pos.Y].Blocked
			== 0) {
		MapData[UserList[UserIndex].Pos.Map][UserList[UserIndex].Pos.X][UserList[UserIndex].Pos.Y].Blocked =
				1;
	} else {
		MapData[UserList[UserIndex].Pos.Map][UserList[UserIndex].Pos.X][UserList[UserIndex].Pos.Y].Blocked =
				0;
	}

	Bloquear(true, UserList[UserIndex].Pos.Map, UserList[UserIndex].Pos.X, UserList[UserIndex].Pos.Y,
			MapData[UserList[UserIndex].Pos.Map][UserList[UserIndex].Pos.X][UserList[UserIndex].Pos.Y].Blocked);
}

/* '' */
/* ' Handles the "KillNPCNoRespawn" message. */
/* ' */
/* ' @param    userIndex The index of the user sending the message. */

void HandleKillNPCNoRespawn(int UserIndex) {
	/* '*************************************************** */
	/* 'Author: Nicolas Matias Gonzalez (NIGO) */
	/* 'Last Modification: 12/30/06 */
	/* ' */
	/* '*************************************************** */
	/* 'Remove packet ID */
	UserList[UserIndex].sockctx->incomingData->ReadByte();

	if (UserTieneAlgunPrivilegios(UserIndex, PlayerType_User, PlayerType_Consejero, PlayerType_SemiDios)) {
		return;
	}

	if (UserList[UserIndex].flags.TargetNPC == 0) {
		return;
	}

	QuitarNPC(UserList[UserIndex].flags.TargetNPC);
	LogGM(UserList[UserIndex].Name, "/MATA " + Npclist[UserList[UserIndex].flags.TargetNPC].Name);
}

/* '' */
/* ' Handles the "KillAllNearbyNPCs" message. */
/* ' */
/* ' @param    userIndex The index of the user sending the message. */

void HandleKillAllNearbyNPCs(int UserIndex) {
	/* '*************************************************** */
	/* 'Author: Nicolas Matias Gonzalez (NIGO) */
	/* 'Last Modification: 12/30/06 */
	/* ' */
	/* '*************************************************** */
	/* 'Remove packet ID */
	UserList[UserIndex].sockctx->incomingData->ReadByte();

	if (UserTieneAlgunPrivilegios(UserIndex, PlayerType_User, PlayerType_Consejero, PlayerType_SemiDios)) {
		return;
	}

	int X;
	int Y;

	for (Y = (UserList[UserIndex].Pos.Y - MinYBorder + 1); Y <= (UserList[UserIndex].Pos.Y + MinYBorder - 1);
			Y++) {
		for (X = (UserList[UserIndex].Pos.X - MinXBorder + 1);
				X <= (UserList[UserIndex].Pos.X + MinXBorder - 1); X++) {
			if (X > 0 && Y > 0 && X < 101 && Y < 101) {
				if (MapData[UserList[UserIndex].Pos.Map][X][Y].NpcIndex > 0) {
					QuitarNPC(MapData[UserList[UserIndex].Pos.Map][X][Y].NpcIndex);
				}
			}
		}
	}
	LogGM(UserList[UserIndex].Name, "/MASSKILL");
}

/* '' */
/* ' Handles the "LastIP" message. */
/* ' */
/* ' @param    userIndex The index of the user sending the message. */

void HandleLastIP(int UserIndex) {
	/* '*************************************************** */
	/* 'Author: Nicolas Matias Gonzalez (NIGO) */
	/* 'Last Modification: 12/30/06 */
	/* ' */
	/* '*************************************************** */
	if (UserList[UserIndex].sockctx->incomingData->length() < 3) {
		throw insufficient_data_error();
		return;
	}

	/* 'This packet contains strings, make a copy of the data to prevent losses if it's not complete yet... */
	clsByteQueue* buffer = UserList[UserIndex].sockctx->incomingData.get();


	/* 'Remove packet ID */
	buffer->ReadByte();

	std::string UserName;
	std::string lista;
	int LoopC;
	unsigned int priv;
	bool validCheck = false;

	priv = PlayerType_Admin | PlayerType_Dios | PlayerType_SemiDios | PlayerType_Consejero;
	UserName = buffer->ReadUnicodeString();

	if (UserTieneAlgunPrivilegios(UserIndex, PlayerType_Admin, PlayerType_Dios, PlayerType_SemiDios)
			&& (!UserTienePrivilegio(UserIndex, PlayerType_RoleMaster))) {
		/* 'Handle special chars */
		if ((vb6::InStrB(UserName, "/") != 0)) {
			UserName = vb6::Replace(UserName, "/", "");
		}
		if ((vb6::InStrB(UserName, "/") != 0)) {
			UserName = vb6::Replace(UserName, "/", "");
		}
		if ((vb6::InStrB(UserName, "+") != 0)) {
			UserName = vb6::Replace(UserName, "+", " ");
		}

		int TargetIndex = NameIndex(UserName);

		/* 'Only Gods and Admins can see the ips of adminsitrative characters. All others can be seen by every adminsitrative char. */
		if (TargetIndex > 0) {
			validCheck = !UserTieneAlgunPrivilegios(TargetIndex, PlayerType_Admin, PlayerType_Dios, PlayerType_SemiDios, PlayerType_Consejero)
					|| (UserTieneAlgunPrivilegios(UserIndex, PlayerType_Admin, PlayerType_Dios));
		} else {
			validCheck = (UserDarPrivilegioLevel(UserName) && priv) == 0
					|| (UserTieneAlgunPrivilegios(UserIndex, PlayerType_Admin, PlayerType_Dios));
		}

		if (validCheck) {
			LogGM(UserList[UserIndex].Name, "/LASTIP " + UserName);

			if (FileExist(GetCharPath(UserName), 0)) {
				lista = "Las ultimas IPs con las que " + UserName + " se conectó son:";
				for (LoopC = (1); LoopC <= (5); LoopC++) {
					lista = lista + vbCrLf + vb6::CStr(LoopC) + " - "
							+ GetVar(GetCharPath(UserName), "INIT", "LastIP" + vb6::CStr(LoopC));
				}
				WriteConsoleMsg(UserIndex, lista, FontTypeNames_FONTTYPE_INFO);
			} else {
				WriteConsoleMsg(UserIndex, "Charfile " "" + UserName + "" " inexistente.",
						FontTypeNames_FONTTYPE_INFO);
			}
		} else {
			WriteConsoleMsg(UserIndex, UserName + " es de mayor jerarquía que vos.",
					FontTypeNames_FONTTYPE_INFO);
		}
	}

	/* 'If we got here then packet is complete, copy data back to original queue */


}

/* '' */
/* ' Handles the "ChatColor" message. */
/* ' */
/* ' @param    userIndex The index of the user sending the message. */

void HandleChatColor(int UserIndex) {
	/* '*************************************************** */
	/* 'Author: Lucas Tavolaro Ortiz (Tavo) */
	/* 'Last Modification: 12/23/06 */
	/* 'Last modified by: Juan Martín Sotuyo Dodero (Maraxus) */
	/* 'Change the user`s chat color */
	/* '*************************************************** */
	if (UserList[UserIndex].sockctx->incomingData->length() < 4) {
		throw insufficient_data_error();
		return;
	}

	/* 'Remove packet ID */
	UserList[UserIndex].sockctx->incomingData->ReadByte();

	int color;

	{
		int r = UserList[UserIndex].sockctx->incomingData->ReadByte();
		int g = UserList[UserIndex].sockctx->incomingData->ReadByte();
		int b = UserList[UserIndex].sockctx->incomingData->ReadByte();
		color = vb6::RGB(r,g,b);
	}

	if (UserTieneAlgunPrivilegios(UserIndex, PlayerType_Admin, PlayerType_Dios, PlayerType_RoleMaster)) {
		UserList[UserIndex].flags.ChatColor = color;
	}
}

/* '' */
/* ' Handles the "Ignored" message. */
/* ' */
/* ' @param    userIndex The index of the user sending the message. */

void HandleIgnored(int UserIndex) {
	/* '*************************************************** */
	/* 'Author: Lucas Tavolaro Ortiz (Tavo) */
	/* 'Last Modification: 12/23/06 */
	/* 'Ignore the user */
	/* '*************************************************** */
	/* 'Remove packet ID */
	UserList[UserIndex].sockctx->incomingData->ReadByte();

	if (UserTieneAlgunPrivilegios(UserIndex, PlayerType_Admin, PlayerType_Dios, PlayerType_SemiDios, PlayerType_Consejero)) {
		UserList[UserIndex].flags.AdminPerseguible = !UserList[UserIndex].flags.AdminPerseguible;
	}
}

/* '' */
/* ' Handles the "CheckSlot" message. */
/* ' */
/* ' @param    userIndex The index of the user sending the message. */

void HandleCheckSlot(int UserIndex) {
	/* '*************************************************** */
	/* 'Author: Pablo (ToxicWaste) */
	/* 'Last Modification: 07/06/2010 */
	/* 'Check one Users Slot in Particular from Inventory */
	/* '07/06/2010: ZaMa - Ahora no se puede usar para saber si hay dioses/admins online. */
	/* '*************************************************** */
	if (UserList[UserIndex].sockctx->incomingData->length() < 4) {
		throw insufficient_data_error();
		return;
	}

	/* 'This packet contains strings, make a copy of the data to prevent losses if it's not complete yet... */
	clsByteQueue* buffer = UserList[UserIndex].sockctx->incomingData.get();


	/* 'Remove packet ID */
	buffer->ReadByte();

	/* 'Reads the UserName and Slot Packets */
	std::string UserName;
	int Slot;
	int tIndex;

	bool UserIsAdmin = false;
	bool OtherUserIsAdmin = false;

	/* 'Que UserName? */
	UserName = buffer->ReadUnicodeString();
	/* 'Que Slot? */
	Slot = buffer->ReadByte();

	UserIsAdmin = UserTieneAlgunPrivilegios(UserIndex, PlayerType_Admin, PlayerType_Dios);

	if (UserTieneAlgunPrivilegios(UserIndex, PlayerType_SemiDios) || UserIsAdmin) {

		LogGM(UserList[UserIndex].Name,
				UserList[UserIndex].Name + " Checkeó el slot " + vb6::CStr(Slot) + " de " + UserName);

		/* 'Que user index? */
		tIndex = NameIndex(UserName);
		OtherUserIsAdmin = EsDios(UserName) || EsAdmin(UserName);

		if (tIndex > 0) {
			if (UserIsAdmin || !OtherUserIsAdmin) {
				if (Slot > 0 && Slot <= UserList[tIndex].CurrentInventorySlots) {
					if (UserList[tIndex].Invent.Object[Slot].ObjIndex > 0) {
						WriteConsoleMsg(UserIndex,
								" Objeto " + vb6::CStr(Slot) + ") "
										+ ObjData[UserList[tIndex].Invent.Object[Slot].ObjIndex].Name
										+ " Cantidad:" + vb6::CStr(UserList[tIndex].Invent.Object[Slot].Amount),
								FontTypeNames_FONTTYPE_INFO);
					} else {
						WriteConsoleMsg(UserIndex, "No hay ningún objeto en slot seleccionado.",
								FontTypeNames_FONTTYPE_INFO);
					}
				} else {
					WriteConsoleMsg(UserIndex, "Slot Inválido.", FontTypeNames_FONTTYPE_TALK);
				}
			} else {
				WriteConsoleMsg(UserIndex, "No puedes ver slots de un dios o admin.",
						FontTypeNames_FONTTYPE_INFO);
			}
		} else {
			if (UserIsAdmin || !OtherUserIsAdmin) {
				WriteConsoleMsg(UserIndex, "Usuario offline.", FontTypeNames_FONTTYPE_TALK);
			} else {
				WriteConsoleMsg(UserIndex, "No puedes ver slots de un dios o admin.",
						FontTypeNames_FONTTYPE_INFO);
			}
		}
	}

	/* 'If we got here then packet is complete, copy data back to original queue */


}

/* '' */
/* ' Handles the "ResetAutoUpdate" message. */
/* ' */
/* ' @param    userIndex The index of the user sending the message. */

void HandleResetAutoUpdate(int UserIndex) {
	/* '*************************************************** */
	/* 'Author: Lucas Tavolaro Ortiz (Tavo) */
	/* 'Last Modification: 12/23/06 */
	/* 'Reset the AutoUpdate */
	/* '*************************************************** */
	/* 'Remove packet ID */
	UserList[UserIndex].sockctx->incomingData->ReadByte();

	if (UserTieneAlgunPrivilegios(UserIndex, PlayerType_User, PlayerType_Consejero, PlayerType_SemiDios)) {
		return;
	}

#if 0
	if (vb6::UCase(UserList[UserIndex].Name) != "MARAXUS") {
		return;
	}
#endif

	WriteConsoleMsg(UserIndex, "No, no no... comando deshabilitado.", FontTypeNames_FONTTYPE_INFO);

	// WriteConsoleMsg(UserIndex, "TID: " + vb6::CStr(ReiniciarAutoUpdate()), FontTypeNames_FONTTYPE_INFO);
}

/* '' */
/* ' Handles the "Restart" message. */
/* ' */
/* ' @param    userIndex The index of the user sending the message. */

void HandleRestart(int UserIndex) {
	/* '*************************************************** */
	/* 'Author: Lucas Tavolaro Ortiz (Tavo) */
	/* 'Last Modification: 12/23/06 */
	/* 'Restart the game */
	/* '*************************************************** */
	/* 'Remove Packet ID */
	UserList[UserIndex].sockctx->incomingData->ReadByte();

	if (UserTieneAlgunPrivilegios(UserIndex, PlayerType_User, PlayerType_Consejero, PlayerType_SemiDios)) {
		return;
	}

//	if (vb6::UCase(UserList[UserIndex].Name) != "MARAXUS") {
//		return;
//	}

	/* 'time and Time BUG! */
	LogGM(UserList[UserIndex].Name, UserList[UserIndex].Name + " reinició el mundo.");

#if 0
	ReiniciarServidor(true);
#endif

	WriteConsoleMsg(UserIndex, "No, no no... comando deshabilitado.", FontTypeNames_FONTTYPE_INFO);
}

/* '' */
/* ' Handles the "ReloadObjects" message. */
/* ' */
/* ' @param    userIndex The index of the user sending the message. */

void HandleReloadObjects(int UserIndex) {
	/* '*************************************************** */
	/* 'Author: Lucas Tavolaro Ortiz (Tavo) */
	/* 'Last Modification: 12/23/06 */
	/* 'Reload the objects */
	/* '*************************************************** */
	/* 'Remove Packet ID */
	UserList[UserIndex].sockctx->incomingData->ReadByte();

	if (UserTieneAlgunPrivilegios(UserIndex, PlayerType_User, PlayerType_Consejero, PlayerType_SemiDios, PlayerType_RoleMaster)) {
		return;
	}

	LogGM(UserList[UserIndex].Name, UserList[UserIndex].Name + " ha recargado los objetos.");

	LoadOBJData();

	WriteConsoleMsg(UserIndex, "Se ha recargado OBJ.dat.", FontTypeNames_FONTTYPE_INFO);
}

/* '' */
/* ' Handles the "ReloadSpells" message. */
/* ' */
/* ' @param    userIndex The index of the user sending the message. */

void HandleReloadSpells(int UserIndex) {
	/* '*************************************************** */
	/* 'Author: Lucas Tavolaro Ortiz (Tavo) */
	/* 'Last Modification: 12/23/06 */
	/* 'Reload the spells */
	/* '*************************************************** */
	/* 'Remove Packet ID */
	UserList[UserIndex].sockctx->incomingData->ReadByte();

	if (UserTieneAlgunPrivilegios(UserIndex, PlayerType_User, PlayerType_Consejero, PlayerType_SemiDios, PlayerType_RoleMaster)) {
		return;
	}

	LogGM(UserList[UserIndex].Name, UserList[UserIndex].Name + " ha recargado los hechizos.");

	CargarHechizos();

	WriteConsoleMsg(UserIndex, "Se ha recargado Hechizos.dat.", FontTypeNames_FONTTYPE_INFO);
}

/* '' */
/* ' Handle the "ReloadServerIni" message. */
/* ' */
/* ' @param userIndex The index of the user sending the message */

void HandleReloadServerIni(int UserIndex) {
	/* '*************************************************** */
	/* 'Author: Lucas Tavolaro Ortiz (Tavo) */
	/* 'Last Modification: 12/23/06 */
	/* 'Reload the Server`s INI */
	/* '*************************************************** */
	/* 'Remove Packet ID */
	UserList[UserIndex].sockctx->incomingData->ReadByte();

	if (UserTieneAlgunPrivilegios(UserIndex, PlayerType_User, PlayerType_Consejero, PlayerType_SemiDios, PlayerType_RoleMaster)) {
		return;
	}

	LogGM(UserList[UserIndex].Name, UserList[UserIndex].Name + " ha recargado los INITs.");

	LoadSini();

	WriteConsoleMsg(UserIndex, "Server.ini actualizado correctamente", FontTypeNames_FONTTYPE_INFO);
}

/* '' */
/* ' Handle the "ReloadNPCs" message */
/* ' */
/* ' @param userIndex The index of the user sending the message */

void HandleReloadNPCs(int UserIndex) {
	/* '*************************************************** */
	/* 'Author: Lucas Tavolaro Ortiz (Tavo) */
	/* 'Last Modification: 12/23/06 */
	/* 'Reload the Server`s NPC */
	/* '*************************************************** */
	/* 'Remove Packet ID */
	UserList[UserIndex].sockctx->incomingData->ReadByte();

	if (UserTieneAlgunPrivilegios(UserIndex, PlayerType_User, PlayerType_Consejero, PlayerType_SemiDios, PlayerType_RoleMaster)) {
		return;
	}

	LogGM(UserList[UserIndex].Name, UserList[UserIndex].Name + " ha recargado los NPCs.");

	CargaNpcsDat();

	WriteConsoleMsg(UserIndex, "Npcs.dat recargado.", FontTypeNames_FONTTYPE_INFO);
}

/* '' */
/* ' Handle the "KickAllChars" message */
/* ' */
/* ' @param userIndex The index of the user sending the message */

void HandleKickAllChars(int UserIndex) {
	/* '*************************************************** */
	/* 'Author: Lucas Tavolaro Ortiz (Tavo) */
	/* 'Last Modification: 12/23/06 */
	/* 'Kick all the chars that are online */
	/* '*************************************************** */
	/* 'Remove Packet ID */
	UserList[UserIndex].sockctx->incomingData->ReadByte();

	if (UserTieneAlgunPrivilegios(UserIndex, PlayerType_User, PlayerType_Consejero, PlayerType_SemiDios, PlayerType_RoleMaster)) {
		return;
	}

	LogGM(UserList[UserIndex].Name, UserList[UserIndex].Name + " ha echado a todos los personajes.");

	EcharPjsNoPrivilegiados();
}

/* '' */
/* ' Handle the "Night" message */
/* ' */
/* ' @param userIndex The index of the user sending the message */

void HandleNight(int UserIndex) {
	/* '*************************************************** */
	/* 'Author: Lucas Tavolaro Ortiz (Tavo) */
	/* 'Last Modification: 12/23/06 */
	/* 'Last modified by: Juan Martín Sotuyo Dodero (Maraxus) */
	/* ' */
	/* '*************************************************** */
	/* 'Remove Packet ID */
	UserList[UserIndex].sockctx->incomingData->ReadByte();

	if (UserTieneAlgunPrivilegios(UserIndex, PlayerType_User, PlayerType_Consejero, PlayerType_SemiDios)) {
		return;
	}

#if 0
	if (vb6::UCase(UserList[UserIndex].Name) != "MARAXUS") {
		return;
	}
#endif

	DeNoche = !DeNoche;

	int i;

	for (i = (1); i <= (NumUsers); i++) {
		if (UserList[i].flags.UserLogged && UserList[i].ConnID > -1) {
			EnviarNoche(i);
		}
	}
}

/* '' */
/* ' Handle the "ShowServerForm" message */
/* ' */
/* ' @param userIndex The index of the user sending the message */

void HandleShowServerForm(int UserIndex) {
	/* '*************************************************** */
	/* 'Author: Lucas Tavolaro Ortiz (Tavo) */
	/* 'Last Modification: 12/23/06 */
	/* 'Show the server form */
	/* '*************************************************** */
	/* 'Remove Packet ID */
	UserList[UserIndex].sockctx->incomingData->ReadByte();

	if (UserTieneAlgunPrivilegios(UserIndex, PlayerType_User, PlayerType_Consejero, PlayerType_SemiDios, PlayerType_RoleMaster)) {
		return;
	}

	LogGM(UserList[UserIndex].Name,
			UserList[UserIndex].Name + " ha solicitado mostrar el formulario del servidor.");
}

/* '' */
/* ' Handle the "CleanSOS" message */
/* ' */
/* ' @param userIndex The index of the user sending the message */

void HandleCleanSOS(int UserIndex) {
	/* '*************************************************** */
	/* 'Author: Lucas Tavolaro Ortiz (Tavo) */
	/* 'Last Modification: 12/23/06 */
	/* 'Clean the SOS */
	/* '*************************************************** */
	/* 'Remove Packet ID */
	UserList[UserIndex].sockctx->incomingData->ReadByte();

	if (UserTieneAlgunPrivilegios(UserIndex, PlayerType_User, PlayerType_Consejero, PlayerType_SemiDios, PlayerType_RoleMaster)) {
		return;
	}

	LogGM(UserList[UserIndex].Name, UserList[UserIndex].Name + " ha borrado los SOS.");

	Ayuda.clear();
}

/* '' */
/* ' Handle the "SaveChars" message */
/* ' */
/* ' @param userIndex The index of the user sending the message */

void HandleSaveChars(int UserIndex) {
	/* '*************************************************** */
	/* 'Author: Lucas Tavolaro Ortiz (Tavo) */
	/* 'Last Modification: 12/23/06 */
	/* 'Save the characters */
	/* '*************************************************** */
	/* 'Remove Packet ID */
	UserList[UserIndex].sockctx->incomingData->ReadByte();

	if (UserTieneAlgunPrivilegios(UserIndex, PlayerType_User, PlayerType_Consejero, PlayerType_SemiDios, PlayerType_RoleMaster)) {
		return;
	}

	LogGM(UserList[UserIndex].Name, UserList[UserIndex].Name + " ha guardado todos los chars.");

	ActualizaExperiencias();
	GuardarUsuarios();
}

/* '' */
/* ' Handle the "ChangeMapInfoBackup" message */
/* ' */
/* ' @param userIndex The index of the user sending the message */

void HandleChangeMapInfoBackup(int UserIndex) {
	/* '*************************************************** */
	/* 'Author: Lucas Tavolaro Ortiz (Tavo) */
	/* 'Last Modification: 12/24/06 */
	/* 'Last modified by: Juan Martín Sotuyo Dodero (Maraxus) */
	/* 'Change the backup`s info of the map */
	/* '*************************************************** */
	if (UserList[UserIndex].sockctx->incomingData->length() < 2) {
		throw insufficient_data_error();
		return;
	}

	/* 'Remove Packet ID */
	UserList[UserIndex].sockctx->incomingData->ReadByte();

	bool doTheBackUp;

	doTheBackUp = UserList[UserIndex].sockctx->incomingData->ReadBoolean();

	if (!UserTieneAlgunPrivilegios(UserIndex, PlayerType_Admin, PlayerType_Dios)) {
		return;
	}

	LogGM(UserList[UserIndex].Name,
			UserList[UserIndex].Name + " ha cambiado la información sobre el BackUp.");

	/* 'Change the boolean to byte in a fast way */
	if (doTheBackUp) {
		MapInfo[UserList[UserIndex].Pos.Map].BackUp = 1;
	} else {
		MapInfo[UserList[UserIndex].Pos.Map].BackUp = 0;
	}

	/* 'Change the boolean to string in a fast way */
	WriteVar(GetMapPath(UserList[UserIndex].Pos.Map, MAPPATH::DAT),
			"Mapa" + vb6::CStr(UserList[UserIndex].Pos.Map), "backup", vb6::CStr(MapInfo[UserList[UserIndex].Pos.Map].BackUp));

	WriteConsoleMsg(UserIndex,
			"Mapa " + vb6::CStr(UserList[UserIndex].Pos.Map) + " Backup: " + vb6::CStr(MapInfo[UserList[UserIndex].Pos.Map].BackUp),
			FontTypeNames_FONTTYPE_INFO);
}

/* '' */
/* ' Handle the "ChangeMapInfoPK" message */
/* ' */
/* ' @param userIndex The index of the user sending the message */

void HandleChangeMapInfoPK(int UserIndex) {
	/* '*************************************************** */
	/* 'Author: Lucas Tavolaro Ortiz (Tavo) */
	/* 'Last Modification: 12/24/06 */
	/* 'Last modified by: Juan Martín Sotuyo Dodero (Maraxus) */
	/* 'Change the pk`s info of the  map */
	/* '*************************************************** */
	if (UserList[UserIndex].sockctx->incomingData->length() < 2) {
		throw insufficient_data_error();
		return;
	}

	/* 'Remove Packet ID */
	UserList[UserIndex].sockctx->incomingData->ReadByte();

	bool isMapPk;

	isMapPk = UserList[UserIndex].sockctx->incomingData->ReadBoolean();

	if (!UserTieneAlgunPrivilegios(UserIndex, PlayerType_Admin, PlayerType_Dios)) {
		return;
	}

	LogGM(UserList[UserIndex].Name,
			UserList[UserIndex].Name + " ha cambiado la información sobre si es PK el mapa.");

	MapInfo[UserList[UserIndex].Pos.Map].Pk = isMapPk;

	/* 'Change the boolean to string in a fast way */
	WriteVar(GetMapPath(UserList[UserIndex].Pos.Map, MAPPATH::DAT),
			"Mapa" + vb6::CStr(UserList[UserIndex].Pos.Map), "Pk", vb6::IIf(isMapPk, "1", "0"));

	WriteConsoleMsg(UserIndex,
			"Mapa " + vb6::CStr(UserList[UserIndex].Pos.Map) + " PK: " + vb6::CStr(MapInfo[UserList[UserIndex].Pos.Map].Pk),
			FontTypeNames_FONTTYPE_INFO);
}

/* '' */
/* ' Handle the "ChangeMapInfoRestricted" message */
/* ' */
/* ' @param userIndex The index of the user sending the message */

void HandleChangeMapInfoRestricted(int UserIndex) {
	/* '*************************************************** */
	/* 'Author: Pablo (ToxicWaste) */
	/* 'Last Modification: 26/01/2007 */
	/* 'Restringido -> Options: "NEWBIE", "NO", "ARMADA", "CAOS", "FACCION". */
	/* '*************************************************** */
	if (UserList[UserIndex].sockctx->incomingData->length() < 3) {
		throw insufficient_data_error();
		return;
	}

	std::string tStr;

	/* 'This packet contains strings, make a copy of the data to prevent losses if it's not complete yet... */
	clsByteQueue* buffer = UserList[UserIndex].sockctx->incomingData.get();


	/* 'Remove Packet ID */
	buffer->ReadByte();

	tStr = buffer->ReadUnicodeString();

	if (UserTieneAlgunPrivilegios(UserIndex, PlayerType_Admin, PlayerType_Dios)) {
		if (tStr == "NEWBIE" || tStr == "NO" || tStr == "ARMADA" || tStr == "CAOS" || tStr == "FACCION") {
			LogGM(UserList[UserIndex].Name,
					UserList[UserIndex].Name
							+ " ha cambiado la información sobre si es restringido el mapa.");

			MapInfo[UserList[UserIndex].Pos.Map].Restringir = RestrictStringToByte(tStr);

			WriteVar(GetMapPath(UserList[UserIndex].Pos.Map, MAPPATH::DAT),
					"Mapa" + vb6::CStr(UserList[UserIndex].Pos.Map), "Restringir", tStr);
			WriteConsoleMsg(UserIndex,
					"Mapa " + vb6::CStr(UserList[UserIndex].Pos.Map) + " Restringido: "
							+ RestrictByteToString(MapInfo[UserList[UserIndex].Pos.Map].Restringir),
					FontTypeNames_FONTTYPE_INFO);
		} else {
			WriteConsoleMsg(UserIndex,
					"Opciones para restringir: 'NEWBIE', 'NO', 'ARMADA', 'CAOS', 'FACCION'",
					FontTypeNames_FONTTYPE_INFO);
		}
	}

	/* 'If we got here then packet is complete, copy data back to original queue */


}

/* '' */
/* ' Handle the "ChangeMapInfoNoMagic" message */
/* ' */
/* ' @param userIndex The index of the user sending the message */

void HandleChangeMapInfoNoMagic(int UserIndex) {
	/* '*************************************************** */
	/* 'Author: Pablo (ToxicWaste) */
	/* 'Last Modification: 26/01/2007 */
	/* 'MagiaSinEfecto -> Options: "1" , "0". */
	/* '*************************************************** */
	if (UserList[UserIndex].sockctx->incomingData->length() < 2) {
		throw insufficient_data_error();
		return;
	}

	bool nomagic;

	/* 'Remove Packet ID */
	UserList[UserIndex].sockctx->incomingData->ReadByte();

	nomagic = UserList[UserIndex].sockctx->incomingData->ReadBoolean();

	if (UserTieneAlgunPrivilegios(UserIndex, PlayerType_Admin, PlayerType_Dios)) {
		LogGM(UserList[UserIndex].Name,
				UserList[UserIndex].Name
						+ " ha cambiado la información sobre si está permitido usar la magia el mapa.");
		MapInfo[UserList[UserIndex].Pos.Map].MagiaSinEfecto = nomagic;
		WriteVar(GetMapPath(UserList[UserIndex].Pos.Map, MAPPATH::DAT),
				"Mapa" + vb6::CStr(UserList[UserIndex].Pos.Map), "MagiaSinEfecto", nomagic);
		WriteConsoleMsg(UserIndex,
				"Mapa " + vb6::CStr(UserList[UserIndex].Pos.Map) + " MagiaSinEfecto: "
						+ vb6::CStr(MapInfo[UserList[UserIndex].Pos.Map].MagiaSinEfecto), FontTypeNames_FONTTYPE_INFO);
	}
}

/* '' */
/* ' Handle the "ChangeMapInfoNoInvi" message */
/* ' */
/* ' @param userIndex The index of the user sending the message */

void HandleChangeMapInfoNoInvi(int UserIndex) {
	/* '*************************************************** */
	/* 'Author: Pablo (ToxicWaste) */
	/* 'Last Modification: 26/01/2007 */
	/* 'InviSinEfecto -> Options: "1", "0" */
	/* '*************************************************** */
	if (UserList[UserIndex].sockctx->incomingData->length() < 2) {
		throw insufficient_data_error();
		return;
	}

	bool noinvi;

	/* 'Remove Packet ID */
	UserList[UserIndex].sockctx->incomingData->ReadByte();

	noinvi = UserList[UserIndex].sockctx->incomingData->ReadBoolean();

	if (UserTieneAlgunPrivilegios(UserIndex, PlayerType_Admin, PlayerType_Dios)) {
		LogGM(UserList[UserIndex].Name,
				UserList[UserIndex].Name
						+ " ha cambiado la información sobre si está permitido usar la invisibilidad en el mapa.");
		MapInfo[UserList[UserIndex].Pos.Map].InviSinEfecto = noinvi;
		WriteVar(GetMapPath(UserList[UserIndex].Pos.Map, MAPPATH::DAT),
				"Mapa" + vb6::CStr(UserList[UserIndex].Pos.Map), "InviSinEfecto", noinvi);
		WriteConsoleMsg(UserIndex,
				"Mapa " + vb6::CStr(UserList[UserIndex].Pos.Map) + " InviSinEfecto: "
						+ vb6::CStr(MapInfo[UserList[UserIndex].Pos.Map].InviSinEfecto), FontTypeNames_FONTTYPE_INFO);
	}
}

/* '' */
/* ' Handle the "ChangeMapInfoNoResu" message */
/* ' */
/* ' @param userIndex The index of the user sending the message */

void HandleChangeMapInfoNoResu(int UserIndex) {
	/* '*************************************************** */
	/* 'Author: Pablo (ToxicWaste) */
	/* 'Last Modification: 26/01/2007 */
	/* 'ResuSinEfecto -> Options: "1", "0" */
	/* '*************************************************** */
	if (UserList[UserIndex].sockctx->incomingData->length() < 2) {
		throw insufficient_data_error();
		return;
	}

	bool noresu;

	/* 'Remove Packet ID */
	UserList[UserIndex].sockctx->incomingData->ReadByte();

	noresu = UserList[UserIndex].sockctx->incomingData->ReadBoolean();

	if (UserTieneAlgunPrivilegios(UserIndex, PlayerType_Admin, PlayerType_Dios)) {
		LogGM(UserList[UserIndex].Name,
				UserList[UserIndex].Name
						+ " ha cambiado la información sobre si está permitido usar el resucitar en el mapa.");
		MapInfo[UserList[UserIndex].Pos.Map].ResuSinEfecto = noresu;
		WriteVar(GetMapPath(UserList[UserIndex].Pos.Map, MAPPATH::DAT),
				"Mapa" + vb6::CStr(UserList[UserIndex].Pos.Map), "ResuSinEfecto", noresu);
		WriteConsoleMsg(UserIndex,
				"Mapa " + vb6::CStr(UserList[UserIndex].Pos.Map) + " ResuSinEfecto: "
						+ vb6::CStr(MapInfo[UserList[UserIndex].Pos.Map].ResuSinEfecto), FontTypeNames_FONTTYPE_INFO);
	}
}

/* '' */
/* ' Handle the "ChangeMapInfoLand" message */
/* ' */
/* ' @param userIndex The index of the user sending the message */

void HandleChangeMapInfoLand(int UserIndex) {
	/* '*************************************************** */
	/* 'Author: Pablo (ToxicWaste) */
	/* 'Last Modification: 26/01/2007 */
	/* 'Terreno -> Opciones: "BOSQUE", "NIEVE", "DESIERTO", "CIUDAD", "CAMPO", "DUNGEON". */
	/* '*************************************************** */
	if (UserList[UserIndex].sockctx->incomingData->length() < 3) {
		throw insufficient_data_error();
		return;
	}

	std::string tStr;

	/* 'This packet contains strings, make a copy of the data to prevent losses if it's not complete yet... */
	clsByteQueue* buffer = UserList[UserIndex].sockctx->incomingData.get();


	/* 'Remove Packet ID */
	buffer->ReadByte();

	tStr = buffer->ReadUnicodeString();

	if (UserTieneAlgunPrivilegios(UserIndex, PlayerType_Admin, PlayerType_Dios)) {
		if (tStr == "BOSQUE" || tStr == "NIEVE" || tStr == "DESIERTO" || tStr == "CIUDAD" || tStr == "CAMPO"
				|| tStr == "DUNGEON") {
			LogGM(UserList[UserIndex].Name,
					UserList[UserIndex].Name + " ha cambiado la información del terreno del mapa.");

			MapInfo[UserList[UserIndex].Pos.Map].Terreno = TerrainStringToByte(tStr);

			WriteVar(GetMapPath(UserList[UserIndex].Pos.Map, MAPPATH::DAT),
					"Mapa" + vb6::CStr(UserList[UserIndex].Pos.Map), "Terreno", tStr);
			WriteConsoleMsg(UserIndex,
					"Mapa " + vb6::CStr(UserList[UserIndex].Pos.Map) + " Terreno: "
							+ vb6::CStr(TerrainByteToString(MapInfo[UserList[UserIndex].Pos.Map].Terreno)),
					FontTypeNames_FONTTYPE_INFO);
		} else {
			WriteConsoleMsg(UserIndex,
					"Opciones para terreno: 'BOSQUE', 'NIEVE', 'DESIERTO', 'CIUDAD', 'CAMPO', 'DUNGEON'",
					FontTypeNames_FONTTYPE_INFO);
			WriteConsoleMsg(UserIndex,
					"Igualmente, el único útil es 'NIEVE' ya que al ingresarlo, la gente muere de frío en el mapa.",
					FontTypeNames_FONTTYPE_INFO);
		}
	}

	/* 'If we got here then packet is complete, copy data back to original queue */


}

/* '' */
/* ' Handle the "ChangeMapInfoZone" message */
/* ' */
/* ' @param userIndex The index of the user sending the message */

void HandleChangeMapInfoZone(int UserIndex) {
	/* '*************************************************** */
	/* 'Author: Pablo (ToxicWaste) */
	/* 'Last Modification: 26/01/2007 */
	/* 'Zona -> Opciones: "BOSQUE", "NIEVE", "DESIERTO", "CIUDAD", "CAMPO", "DUNGEON". */
	/* '*************************************************** */
	if (UserList[UserIndex].sockctx->incomingData->length() < 3) {
		throw insufficient_data_error();
		return;
	}

	std::string tStr;

	/* 'This packet contains strings, make a copy of the data to prevent losses if it's not complete yet... */
	clsByteQueue* buffer = UserList[UserIndex].sockctx->incomingData.get();


	/* 'Remove Packet ID */
	buffer->ReadByte();

	tStr = buffer->ReadUnicodeString();

	if (UserTieneAlgunPrivilegios(UserIndex, PlayerType_Admin, PlayerType_Dios)) {
		if (tStr == "BOSQUE" || tStr == "NIEVE" || tStr == "DESIERTO" || tStr == "CIUDAD" || tStr == "CAMPO"
				|| tStr == "DUNGEON") {
			LogGM(UserList[UserIndex].Name,
					UserList[UserIndex].Name + " ha cambiado la información de la zona del mapa.");
			MapInfo[UserList[UserIndex].Pos.Map].Zona = tStr;
			WriteVar(GetMapPath(UserList[UserIndex].Pos.Map, MAPPATH::DAT),
					"Mapa" + vb6::CStr(UserList[UserIndex].Pos.Map), "Zona", tStr);
			WriteConsoleMsg(UserIndex,
					"Mapa " + vb6::CStr(UserList[UserIndex].Pos.Map) + " Zona: "
							+ vb6::CStr(MapInfo[UserList[UserIndex].Pos.Map].Zona), FontTypeNames_FONTTYPE_INFO);
		} else {
			WriteConsoleMsg(UserIndex,
					"Opciones para terreno: 'BOSQUE', 'NIEVE', 'DESIERTO', 'CIUDAD', 'CAMPO', 'DUNGEON'",
					FontTypeNames_FONTTYPE_INFO);
			WriteConsoleMsg(UserIndex,
					"Igualmente, el único útil es 'DUNGEON' ya que al ingresarlo, NO se sentirá el efecto de la lluvia en este mapa.",
					FontTypeNames_FONTTYPE_INFO);
		}
	}

	/* 'If we got here then packet is complete, copy data back to original queue */


}

/* '' */
/* ' Handle the "ChangeMapInfoStealNp" message */
/* ' */
/* ' @param userIndex The index of the user sending the message */

void HandleChangeMapInfoStealNpc(int UserIndex) {
	/* '*************************************************** */
	/* 'Author: ZaMa */
	/* 'Last Modification: 25/07/2010 */
	/* 'RoboNpcsPermitido -> Options: "1", "0" */
	/* '*************************************************** */
	if (UserList[UserIndex].sockctx->incomingData->length() < 2) {
		throw insufficient_data_error();
		return;
	}

	int RoboNpc;

	/* 'Remove Packet ID */
	UserList[UserIndex].sockctx->incomingData->ReadByte();

	RoboNpc = (vb6::IIf(UserList[UserIndex].sockctx->incomingData->ReadBoolean(), 1, 0));

	if (UserTieneAlgunPrivilegios(UserIndex, PlayerType_Admin, PlayerType_Dios)) {
		LogGM(UserList[UserIndex].Name,
				UserList[UserIndex].Name
						+ " ha cambiado la información sobre si está permitido robar npcs en el mapa.");

		MapInfo[UserList[UserIndex].Pos.Map].RoboNpcsPermitido = RoboNpc;

		WriteVar(GetMapPath(UserList[UserIndex].Pos.Map, MAPPATH::DAT),
				"Mapa" + vb6::CStr(UserList[UserIndex].Pos.Map), "RoboNpcsPermitido", RoboNpc);
		WriteConsoleMsg(UserIndex,
				"Mapa " + vb6::CStr(UserList[UserIndex].Pos.Map) + " RoboNpcsPermitido: "
						+ vb6::CStr(MapInfo[UserList[UserIndex].Pos.Map].RoboNpcsPermitido),
				FontTypeNames_FONTTYPE_INFO);
	}
}

/* '' */
/* ' Handle the "ChangeMapInfoNoOcultar" message */
/* ' */
/* ' @param userIndex The index of the user sending the message */

void HandleChangeMapInfoNoOcultar(int UserIndex) {
	/* '*************************************************** */
	/* 'Author: ZaMa */
	/* 'Last Modification: 18/09/2010 */
	/* 'OcultarSinEfecto -> Options: "1", "0" */
	/* '*************************************************** */
	if (UserList[UserIndex].sockctx->incomingData->length() < 2) {
		throw insufficient_data_error();
		return;
	}

	int NoOcultar;
	int mapa;

	/* 'Remove Packet ID */
	UserList[UserIndex].sockctx->incomingData->ReadByte();

	NoOcultar = (vb6::IIf(UserList[UserIndex].sockctx->incomingData->ReadBoolean(), 1, 0));

	if (UserTieneAlgunPrivilegios(UserIndex, PlayerType_Admin, PlayerType_Dios)) {

		mapa = UserList[UserIndex].Pos.Map;

		LogGM(UserList[UserIndex].Name,
				UserList[UserIndex].Name
						+ " ha cambiado la información sobre si está permitido ocultarse en el mapa " + vb6::CStr(mapa)
						+ ".");

		MapInfo[mapa].OcultarSinEfecto = NoOcultar;

		WriteVar(GetMapPath(mapa, MAPPATH::DAT), "Mapa" + vb6::CStr(mapa), "OcultarSinEfecto",
				vb6::CStr(NoOcultar));
		WriteConsoleMsg(UserIndex, "Mapa " + vb6::CStr(mapa) + " OcultarSinEfecto: " + vb6::CStr(NoOcultar),
				FontTypeNames_FONTTYPE_INFO);
	}

}

/* '' */
/* ' Handle the "ChangeMapInfoNoInvocar" message */
/* ' */
/* ' @param userIndex The index of the user sending the message */

void HandleChangeMapInfoNoInvocar(int UserIndex) {
	/* '*************************************************** */
	/* 'Author: ZaMa */
	/* 'Last Modification: 18/09/2010 */
	/* 'InvocarSinEfecto -> Options: "1", "0" */
	/* '*************************************************** */
	if (UserList[UserIndex].sockctx->incomingData->length() < 2) {
		throw insufficient_data_error();
		return;
	}

	int NoInvocar;
	int mapa;

	/* 'Remove Packet ID */
	UserList[UserIndex].sockctx->incomingData->ReadByte();

	NoInvocar = (vb6::IIf(UserList[UserIndex].sockctx->incomingData->ReadBoolean(), 1, 0));

	if (UserTieneAlgunPrivilegios(UserIndex, PlayerType_Admin, PlayerType_Dios)) {

		mapa = UserList[UserIndex].Pos.Map;

		LogGM(UserList[UserIndex].Name,
				UserList[UserIndex].Name
						+ " ha cambiado la información sobre si está permitido invocar en el mapa " + vb6::CStr(mapa)
						+ ".");

		MapInfo[mapa].InvocarSinEfecto = NoInvocar;

		WriteVar(GetMapPath(mapa, MAPPATH::DAT), "Mapa" + vb6::CStr(mapa), "InvocarSinEfecto",
				NoInvocar);
		WriteConsoleMsg(UserIndex, "Mapa " + vb6::CStr(mapa) + " InvocarSinEfecto: " + vb6::CStr(NoInvocar),
				FontTypeNames_FONTTYPE_INFO);
	}

}

/* '' */
/* ' Handle the "SaveMap" message */
/* ' */
/* ' @param userIndex The index of the user sending the message */

void HandleSaveMap(int UserIndex) {
	/* '*************************************************** */
	/* 'Author: Lucas Tavolaro Ortiz (Tavo) */
	/* 'Last Modification: 12/24/06 */
	/* 'Saves the map */
	/* '*************************************************** */
	/* 'Remove Packet ID */
	UserList[UserIndex].sockctx->incomingData->ReadByte();

	if (!UserTieneAlgunPrivilegios(UserIndex, PlayerType_Admin, PlayerType_Dios)) {
		return;
	}

	LogGM(UserList[UserIndex].Name,
			UserList[UserIndex].Name + " ha guardado el mapa " + vb6::CStr(UserList[UserIndex].Pos.Map));

	GrabarMapa(UserList[UserIndex].Pos.Map, true);

	WriteConsoleMsg(UserIndex, "Mapa Guardado.", FontTypeNames_FONTTYPE_INFO);
}

/* '' */
/* ' Handle the "ShowGuildMessages" message */
/* ' */
/* ' @param userIndex The index of the user sending the message */

void HandleShowGuildMessages(int UserIndex) {
	/* '*************************************************** */
	/* 'Author: Lucas Tavolaro Ortiz (Tavo) */
	/* 'Last Modification: 12/24/06 */
	/* 'Last modified by: Juan Martín Sotuyo Dodero (Maraxus) */
	/* 'Allows admins to read guild messages */
	/* '*************************************************** */
	if (UserList[UserIndex].sockctx->incomingData->length() < 3) {
		throw insufficient_data_error();
		return;
	}

	/* 'This packet contains strings, make a copy of the data to prevent losses if it's not complete yet... */
	clsByteQueue* buffer = UserList[UserIndex].sockctx->incomingData.get();


	/* 'Remove packet ID */
	buffer->ReadByte();

	std::string guild;

	guild = buffer->ReadUnicodeString();

	if (!UserTienePrivilegio(UserIndex, PlayerType_RoleMaster) && UserTieneAlgunPrivilegios(UserIndex, PlayerType_Admin, PlayerType_Dios)) {
		GMEscuchaClan(UserIndex, guild);
	}

	/* 'If we got here then packet is complete, copy data back to original queue */


}

/* '' */
/* ' Handle the "DoBackUp" message */
/* ' */
/* ' @param userIndex The index of the user sending the message */

void HandleDoBackUp(int UserIndex) {
	/* '*************************************************** */
	/* 'Author: Lucas Tavolaro Ortiz (Tavo) */
	/* 'Last Modification: 12/24/06 */
	/* 'Show guilds messages */
	/* '*************************************************** */
	/* 'Remove Packet ID */
	UserList[UserIndex].sockctx->incomingData->ReadByte();

	if (!UserTieneAlgunPrivilegios(UserIndex, PlayerType_Admin, PlayerType_Dios)) {
		return;
	}

	LogGM(UserList[UserIndex].Name, UserList[UserIndex].Name + " ha hecho un backup.");

	/* 'Sino lo confunde con la id del paquete */
	DoBackUp();
}

/* '' */
/* ' Handle the "ToggleCentinelActivated" message */
/* ' */
/* ' @param userIndex The index of the user sending the message */

void HandleToggleCentinelActivated(int UserIndex) {
	/* '*************************************************** */
	/* 'Author: Lucas Tavolaro Ortiz (Tavo) */
	/* 'Last Modification: 12/26/06 */
	/* 'Last modified by: Juan Martín Sotuyo Dodero (Maraxus) */
	/* 'Activate or desactivate the Centinel */
	/* '*************************************************** */
	/* 'Remove Packet ID */
	UserList[UserIndex].sockctx->incomingData->ReadByte();

	if (UserTieneAlgunPrivilegios(UserIndex, PlayerType_User, PlayerType_Consejero, PlayerType_SemiDios)) {
		return;
	}

	centinelaActivado = !centinelaActivado;

	ResetCentinelas();

	if (centinelaActivado) {
		SendData(SendTarget_ToAdmins, 0,
				PrepareMessageConsoleMsg("El centinela ha sido activado.", FontTypeNames_FONTTYPE_SERVER));
	} else {
		SendData(SendTarget_ToAdmins, 0,
				PrepareMessageConsoleMsg("El centinela ha sido desactivado.", FontTypeNames_FONTTYPE_SERVER));
	}
}

/* '' */
/* ' Handle the "AlterName" message */
/* ' */
/* ' @param userIndex The index of the user sending the message */

void HandleAlterName(int UserIndex) {
	/* '*************************************************** */
	/* 'Author: Juan Martín Sotuyo Dodero (Maraxus) */
	/* 'Last Modification: 12/26/06 */
	/* 'Change user name */
	/* '*************************************************** */
	if (UserList[UserIndex].sockctx->incomingData->length() < 5) {
		throw insufficient_data_error();
		return;
	}

	/* 'This packet contains strings, make a copy of the data to prevent losses if it's not complete yet... */
	clsByteQueue* buffer = UserList[UserIndex].sockctx->incomingData.get();


	/* 'Remove packet ID */
	buffer->ReadByte();

	/* 'Reads the userName and newUser Packets */
	std::string UserName;
	std::string newName;
	int changeNameUI;
	int GuildIndex;

	UserName = buffer->ReadUnicodeString();
	newName = buffer->ReadUnicodeString();

	if (UserTieneAlgunPrivilegios(UserIndex, PlayerType_Admin, PlayerType_Dios)
			|| UserList[UserIndex].flags.PrivEspecial) {
		if (vb6::LenB(UserName) == 0 || vb6::LenB(newName) == 0) {
			WriteConsoleMsg(UserIndex, "Usar: /ANAME origen@destino", FontTypeNames_FONTTYPE_INFO);
		} else {
			changeNameUI = NameIndex(UserName);

			if (changeNameUI > 0) {
				WriteConsoleMsg(UserIndex, "El Pj está online, debe salir para hacer el cambio.",
						FontTypeNames_FONTTYPE_WARNING);
			} else {
				if (!FileExist(GetCharPath(UserName))) {
					WriteConsoleMsg(UserIndex, "El pj " + UserName + " es inexistente.",
							FontTypeNames_FONTTYPE_INFO);
				} else {
					GuildIndex = vb6::val(GetVar(GetCharPath(UserName), "GUILD", "GUILDINDEX"));

					if (GuildIndex > 0) {
						WriteConsoleMsg(UserIndex,
								"El pj " + UserName
										+ " pertenece a un clan, debe salir del mismo con /salirclan para ser transferido.",
								FontTypeNames_FONTTYPE_INFO);
					} else {
						if (!FileExist(GetCharPath(UserName))) {
							vb6::FileCopy(GetCharPath(UserName),
									GetCharPath(newName));

							WriteConsoleMsg(UserIndex, "Transferencia exitosa.", FontTypeNames_FONTTYPE_INFO);

							WriteVar(GetCharPath(UserName), "FLAGS", "Ban", "1");

							int cantPenas;

							cantPenas = vb6::val(GetVar(GetCharPath(UserName), "PENAS", "Cant"));

							WriteVar(GetCharPath(UserName), "PENAS", "Cant", vb6::CStr(cantPenas + 1));

							WriteVar(GetCharPath(UserName), "PENAS", "P" + vb6::CStr(cantPenas + 1),
									vb6::LCase(UserList[UserIndex].Name) + ": BAN POR Cambio de nick a "
											+ vb6::UCase(newName) + " " + vb6::CStr(vb6::Now()));

							LogGM(UserList[UserIndex].Name,
									"Ha cambiado de nombre al usuario " + UserName + ". Ahora se llama "
											+ newName);
						} else {
							WriteConsoleMsg(UserIndex, "El nick solicitado ya existe.",
									FontTypeNames_FONTTYPE_INFO);
						}
					}
				}
			}
		}
	}

	/* 'If we got here then packet is complete, copy data back to original queue */

}

/* '' */
/* ' Handle the "AlterName" message */
/* ' */
/* ' @param userIndex The index of the user sending the message */

void HandleAlterMail(int UserIndex) {
	/* '*************************************************** */
	/* 'Author: Juan Martín Sotuyo Dodero (Maraxus) */
	/* 'Last Modification: 12/26/06 */
	/* 'Change user password */
	/* '*************************************************** */
	if (UserList[UserIndex].sockctx->incomingData->length() < 5) {
		throw insufficient_data_error();
		return;
	}

	/* 'This packet contains strings, make a copy of the data to prevent losses if it's not complete yet... */
	clsByteQueue* buffer = UserList[UserIndex].sockctx->incomingData.get();


	/* 'Remove packet ID */
	buffer->ReadByte();

	std::string UserName;
	std::string newMail;

	UserName = buffer->ReadUnicodeString();
	newMail = buffer->ReadUnicodeString();

	if (!UserTienePrivilegio(UserIndex, PlayerType_RoleMaster) &&
			UserTieneAlgunPrivilegios(UserIndex, PlayerType_Admin, PlayerType_Dios)) {
		if (vb6::LenB(UserName) == 0 || vb6::LenB(newMail) == 0) {
			WriteConsoleMsg(UserIndex, "usar /AEMAIL <pj>-<nuevomail>", FontTypeNames_FONTTYPE_INFO);
		} else {
			if (!FileExist(GetCharPath(UserName))) {
				WriteConsoleMsg(UserIndex, "No existe el charfile " + UserName + ".chr",
						FontTypeNames_FONTTYPE_INFO);
			} else {
				WriteVar(GetCharPath(UserName), "CONTACTO", "Email", newMail);
				WriteConsoleMsg(UserIndex, "Email de " + UserName + " cambiado a: " + newMail,
						FontTypeNames_FONTTYPE_INFO);
			}

			LogGM(UserList[UserIndex].Name, "Le ha cambiado el mail a " + UserName);
		}
	}

	/* 'If we got here then packet is complete, copy data back to original queue */


}

/* '' */
/* ' Handle the "AlterPassword" message */
/* ' */
/* ' @param userIndex The index of the user sending the message */

void HandleAlterPassword(int UserIndex) {
	/* '*************************************************** */
	/* 'Author: Juan Martín Sotuyo Dodero (Maraxus) */
	/* 'Last Modification: 12/26/06 */
	/* 'Change user password */
	/* '*************************************************** */
	if (UserList[UserIndex].sockctx->incomingData->length() < 5) {
		throw insufficient_data_error();
		return;
	}

	/* 'This packet contains strings, make a copy of the data to prevent losses if it's not complete yet... */
	clsByteQueue* buffer = UserList[UserIndex].sockctx->incomingData.get();


	/* 'Remove packet ID */
	buffer->ReadByte();

	std::string UserName;
	std::string copyFrom;
	std::string Password;
	int dummy = 0;

	UserName = vb6::Replace(buffer->ReadUnicodeString(), "+", " ");
	copyFrom = vb6::Replace(buffer->ReadUnicodeString(), "+", " ");

	if (!UserTienePrivilegio(UserIndex, PlayerType_RoleMaster) &&
			UserTieneAlgunPrivilegios(UserIndex, PlayerType_Admin, PlayerType_Dios)) {
		LogGM(UserList[UserIndex].Name, "Ha alterado la contrasena de " + UserName);

		if (vb6::LenB(UserName) == 0 || vb6::LenB(copyFrom) == 0) {
			WriteConsoleMsg(UserIndex, "usar /APASS <pjsinpass>@<pjconpass>", FontTypeNames_FONTTYPE_INFO);
		} else {
			if (!FileExist(GetCharPath(UserName)) || !FileExist(GetCharPath(copyFrom))) {
				WriteConsoleMsg(UserIndex, "Alguno de los PJs no existe " + UserName + "@" + copyFrom,
						FontTypeNames_FONTTYPE_INFO);
			} else {
				if (CheckForSameName(UserName, dummy)) {
					WriteConsoleMsg(UserIndex, "ATENCION: el usuario " + UserName + " se encuentra ONLINE.",
											FontTypeNames_FONTTYPE_INFO);
				}

				WriteSaltedPasswordUserCopyFrom(UserName, copyFrom);

				WriteConsoleMsg(UserIndex, "Password de " + UserName + " ha cambiado por la de " + copyFrom,
						FontTypeNames_FONTTYPE_INFO);
			}
		}
	}

	/* 'If we got here then packet is complete, copy data back to original queue */


}

/* '' */
/* ' Handle the "HandleCreateNPC" message */
/* ' */
/* ' @param userIndex The index of the user sending the message */

void HandleCreateNPC(int UserIndex) {
	/* '*************************************************** */
	/* 'Author: Juan Martín Sotuyo Dodero (Maraxus) */
	/* 'Last Modification: 26/09/2010 */
	/* '26/09/2010: ZaMa - Ya no se pueden crear npcs pretorianos. */
	/* '*************************************************** */
	if (UserList[UserIndex].sockctx->incomingData->length() < 3) {
		throw insufficient_data_error();
		return;
	}

	/* 'Remove Packet ID */
	UserList[UserIndex].sockctx->incomingData->ReadByte();

	int NpcIndex;

	NpcIndex = UserList[UserIndex].sockctx->incomingData->ReadInteger();

	if (UserTieneAlgunPrivilegios(UserIndex, PlayerType_User, PlayerType_Consejero, PlayerType_SemiDios)) {
		return;
	}

	/* FIXME: PRETORIANOS */
#if 0
	if (NpcIndex >= 900) {
		WriteConsoleMsg(UserIndex,
				"No puedes sumonear miembros del clan pretoriano de esta forma, utiliza /CrearClanPretoriano.",
				FontTypeNames_FONTTYPE_WARNING);
		return;
	}
#endif

	NpcIndex = SpawnNpc(NpcIndex, UserList[UserIndex].Pos, true, false);

	if (NpcIndex != 0) {
		LogGM(UserList[UserIndex].Name,
				"Sumoneó a " + Npclist[NpcIndex].Name + " en mapa " + vb6::CStr(UserList[UserIndex].Pos.Map));
	}
}

/* '' */
/* ' Handle the "CreateNPCWithRespawn" message */
/* ' */
/* ' @param userIndex The index of the user sending the message */

void HandleCreateNPCWithRespawn(int UserIndex) {
	/* '*************************************************** */
	/* 'Author: Juan Martín Sotuyo Dodero (Maraxus) */
	/* 'Last Modification: 26/09/2010 */
	/* '26/09/2010: ZaMa - Ya no se pueden crear npcs pretorianos. */
	/* '*************************************************** */
	if (UserList[UserIndex].sockctx->incomingData->length() < 3) {
		throw insufficient_data_error();
		return;
	}

	/* 'Remove Packet ID */
	UserList[UserIndex].sockctx->incomingData->ReadByte();

	int NpcIndex;

	NpcIndex = UserList[UserIndex].sockctx->incomingData->ReadInteger();

	if (NpcIndex >= 900) {
		WriteConsoleMsg(UserIndex,
				"No puedes sumonear miembros del clan pretoriano de esta forma, utiliza /CrearClanPretoriano.",
				FontTypeNames_FONTTYPE_WARNING);
		return;
	}

	if (UserTieneAlgunPrivilegios(UserIndex, PlayerType_User, PlayerType_Consejero, PlayerType_SemiDios, PlayerType_RoleMaster)) {
		return;
	}

	NpcIndex = SpawnNpc(NpcIndex, UserList[UserIndex].Pos, true, true);

	if (NpcIndex != 0) {
		LogGM(UserList[UserIndex].Name,
				"Sumoneó con respawn " + Npclist[NpcIndex].Name + " en mapa " + vb6::CStr(UserList[UserIndex].Pos.Map));
	}
}

/* '' */
/* ' Handle the "ImperialArmour" message */
/* ' */
/* ' @param userIndex The index of the user sending the message */

void HandleImperialArmour(int UserIndex) {
	/* '*************************************************** */
	/* 'Author: Juan Martín Sotuyo Dodero (Maraxus) */
	/* 'Last Modification: 12/24/06 */
	/* ' */
	/* '*************************************************** */
	if (UserList[UserIndex].sockctx->incomingData->length() < 4) {
		throw insufficient_data_error();
		return;
	}

	/* 'Remove Packet ID */
	UserList[UserIndex].sockctx->incomingData->ReadByte();

	int index;
	int ObjIndex;

	index = UserList[UserIndex].sockctx->incomingData->ReadByte();
	ObjIndex = UserList[UserIndex].sockctx->incomingData->ReadInteger();

	if (UserTieneAlgunPrivilegios(UserIndex, PlayerType_User, PlayerType_Consejero, PlayerType_SemiDios, PlayerType_RoleMaster)) {
		return;
	}

	switch (index) {
	case 1:
		ArmaduraImperial1 = ObjIndex;

		break;

	case 2:
		ArmaduraImperial2 = ObjIndex;

		break;

	case 3:
		ArmaduraImperial3 = ObjIndex;

		break;

	case 4:
		TunicaMagoImperial = ObjIndex;
		break;
	}
}

/* '' */
/* ' Handle the "ChaosArmour" message */
/* ' */
/* ' @param userIndex The index of the user sending the message */

void HandleChaosArmour(int UserIndex) {
	/* '*************************************************** */
	/* 'Author: Juan Martín Sotuyo Dodero (Maraxus) */
	/* 'Last Modification: 12/24/06 */
	/* ' */
	/* '*************************************************** */
	if (UserList[UserIndex].sockctx->incomingData->length() < 4) {
		throw insufficient_data_error();
		return;
	}

	/* 'Remove Packet ID */
	UserList[UserIndex].sockctx->incomingData->ReadByte();

	int index;
	int ObjIndex;

	index = UserList[UserIndex].sockctx->incomingData->ReadByte();
	ObjIndex = UserList[UserIndex].sockctx->incomingData->ReadInteger();

	if (UserTieneAlgunPrivilegios(UserIndex, PlayerType_User, PlayerType_Consejero, PlayerType_SemiDios, PlayerType_RoleMaster)) {
		return;
	}

	switch (index) {
	case 1:
		ArmaduraCaos1 = ObjIndex;

		break;

	case 2:
		ArmaduraCaos2 = ObjIndex;

		break;

	case 3:
		ArmaduraCaos3 = ObjIndex;

		break;

	case 4:
		TunicaMagoCaos = ObjIndex;
		break;
	}
}

/* '' */
/* ' Handle the "NavigateToggle" message */
/* ' */
/* ' @param userIndex The index of the user sending the message */

void HandleNavigateToggle(int UserIndex) {
	/* '*************************************************** */
	/* 'Author: Juan Martín Sotuyo Dodero (Maraxus) */
	/* 'Last Modification: 01/12/07 */
	/* ' */
	/* '*************************************************** */
	/* 'Remove Packet ID */
	UserList[UserIndex].sockctx->incomingData->ReadByte();

	if (UserTieneAlgunPrivilegios(UserIndex, PlayerType_User, PlayerType_Consejero)) {
		return;
	}

	if (UserList[UserIndex].flags.Navegando == 1) {
		UserList[UserIndex].flags.Navegando = 0;
	} else {
		UserList[UserIndex].flags.Navegando = 1;
	}

	/* 'Tell the client that we are navigating. */
	WriteNavigateToggle(UserIndex);
}

/* '' */
/* ' Handle the "ServerOpenToUsersToggle" message */
/* ' */
/* ' @param userIndex The index of the user sending the message */

void HandleServerOpenToUsersToggle(int UserIndex) {
	/* '*************************************************** */
	/* 'Author: Juan Martín Sotuyo Dodero (Maraxus) */
	/* 'Last Modification: 12/24/06 */
	/* ' */
	/* '*************************************************** */
	/* 'Remove Packet ID */
	UserList[UserIndex].sockctx->incomingData->ReadByte();

	if (UserTieneAlgunPrivilegios(UserIndex, PlayerType_User, PlayerType_Consejero, PlayerType_SemiDios, PlayerType_RoleMaster)) {
		return;
	}

	if (ServerSoloGMs > 0) {
		WriteConsoleMsg(UserIndex, "Servidor habilitado para todos.", FontTypeNames_FONTTYPE_INFO);
		ServerSoloGMs = 0;
	} else {
		WriteConsoleMsg(UserIndex, "Servidor restringido a administradores.", FontTypeNames_FONTTYPE_INFO);
		ServerSoloGMs = 1;
	}
}

/* '' */
/* ' Handle the "TurnOffServer" message */
/* ' */
/* ' @param userIndex The index of the user sending the message */

void HandleTurnOffServer(int UserIndex) {
	/* '*************************************************** */
	/* 'Author: Juan Martín Sotuyo Dodero (Maraxus) */
	/* 'Last Modification: 12/24/06 */
	/* 'Turns off the server */
	/* '*************************************************** */

	/* 'Remove Packet ID */
	UserList[UserIndex].sockctx->incomingData->ReadByte();

	if (UserTieneAlgunPrivilegios(UserIndex, PlayerType_User, PlayerType_Consejero, PlayerType_SemiDios,
			PlayerType_RoleMaster)) {
		return;
	}

	LogGM(UserList[UserIndex].Name, "/APAGAR");
	SendData(SendTarget_ToAll, 0,
			PrepareMessageConsoleMsg("¡¡¡" + UserList[UserIndex].Name + " VA A APAGAR EL SERVIDOR!!!",
					FontTypeNames_FONTTYPE_FIGHT));

	LogMain(vb6::string_format("%d server apagado por %s", vb6::Now(), UserList[UserIndex].Name.c_str()));
}

/* '' */
/* ' Handle the "TurnCriminal" message */
/* ' */
/* ' @param userIndex The index of the user sending the message */

void HandleTurnCriminal(int UserIndex) {
	/* '*************************************************** */
	/* 'Author: Juan Martín Sotuyo Dodero (Maraxus) */
	/* 'Last Modification: 12/26/06 */
	/* ' */
	/* '*************************************************** */
	if (UserList[UserIndex].sockctx->incomingData->length() < 3) {
		throw insufficient_data_error();
		return;
	}

	/* 'This packet contains strings, make a copy of the data to prevent losses if it's not complete yet... */
	clsByteQueue* buffer = UserList[UserIndex].sockctx->incomingData.get();


	/* 'Remove packet ID */
	buffer->ReadByte();

	std::string UserName;
	int tUser = 0;

	UserName = buffer->ReadUnicodeString();

	if (!UserTieneAlgunPrivilegios(UserIndex, PlayerType_RoleMaster)
			&& UserTieneAlgunPrivilegios(UserIndex, PlayerType_Admin, PlayerType_Dios)) {

		LogGM(UserList[UserIndex].Name, "/CONDEN " + UserName);

		tUser = NameIndex(UserName);
		if (tUser > 0) {
			VolverCriminal(tUser);
		}
	}

	/* 'If we got here then packet is complete, copy data back to original queue */


}

/* '' */
/* ' Handle the "ResetFactions" message */
/* ' */
/* ' @param userIndex The index of the user sending the message */

void HandleResetFactions(int UserIndex) {
	/* '*************************************************** */
	/* 'Author: Juan Martín Sotuyo Dodero (Maraxus) */
	/* 'Last Modification: 06/09/09 */
	/* ' */
	/* '*************************************************** */
	if (UserList[UserIndex].sockctx->incomingData->length() < 3) {
		throw insufficient_data_error();
		return;
	}

	/* 'This packet contains strings, make a copy of the data to prevent losses if it's not complete yet... */
	clsByteQueue* buffer = UserList[UserIndex].sockctx->incomingData.get();


	/* 'Remove packet ID */
	buffer->ReadByte();

	std::string UserName;
	std::string Reason;
	int tUser = 0;
	std::string Char;
	int cantPenas;

	UserName = buffer->ReadUnicodeString();

	if (UserTieneAlgunPrivilegios(UserIndex, PlayerType_Admin, PlayerType_Dios)
			|| UserList[UserIndex].flags.PrivEspecial) {
		LogGM(UserList[UserIndex].Name, "/RAJAR " + UserName);

		tUser = NameIndex(UserName);

		if (tUser > 0) {
			ResetFacciones(tUser);

			cantPenas = vb6::val(GetVar(GetCharPath(UserName), "PENAS", "Cant"));
			WriteVar(GetCharPath(UserName), "PENAS", "Cant", cantPenas + 1);
			WriteVar(GetCharPath(UserName), "PENAS", "P" + vb6::CStr(cantPenas + 1),
					vb6::LCase(UserList[UserIndex].Name) + ": Personaje reincorporado a la facción. "
							+ vb6::CStr(vb6::Now()));
		} else {
			Char = GetCharPath(UserName);

			if (FileExist(Char, 0)) {
				WriteVar(Char, "FACCIONES", "EjercitoReal", 0);
				WriteVar(Char, "FACCIONES", "CiudMatados", 0);
				WriteVar(Char, "FACCIONES", "CrimMatados", 0);
				WriteVar(Char, "FACCIONES", "EjercitoCaos", 0);
				WriteVar(Char, "FACCIONES", "FechaIngreso", "No ingresó a ninguna Facción");
				WriteVar(Char, "FACCIONES", "rArCaos", 0);
				WriteVar(Char, "FACCIONES", "rArReal", 0);
				WriteVar(Char, "FACCIONES", "rExCaos", 0);
				WriteVar(Char, "FACCIONES", "rExReal", 0);
				WriteVar(Char, "FACCIONES", "recCaos", 0);
				WriteVar(Char, "FACCIONES", "recReal", 0);
				WriteVar(Char, "FACCIONES", "Reenlistadas", 0);
				WriteVar(Char, "FACCIONES", "NivelIngreso", 0);
				WriteVar(Char, "FACCIONES", "MatadosIngreso", 0);
				WriteVar(Char, "FACCIONES", "NextRecompensa", 0);

				cantPenas = vb6::val(GetVar(GetCharPath(UserName), "PENAS", "Cant"));
				WriteVar(GetCharPath(UserName), "PENAS", "Cant", cantPenas + 1);
				WriteVar(GetCharPath(UserName), "PENAS", "P" + vb6::CStr(cantPenas + 1),
						vb6::LCase(UserList[UserIndex].Name) + ": Personaje reincorporado a la facción. "
								+ vb6::CStr(vb6::Now()));
			} else {
				WriteConsoleMsg(UserIndex, "El personaje " + UserName + " no existe.",
						FontTypeNames_FONTTYPE_INFO);
			}
		}
	}

	/* 'If we got here then packet is complete, copy data back to original queue */


}

/* '' */
/* ' Handle the "RemoveCharFromGuild" message */
/* ' */
/* ' @param userIndex The index of the user sending the message */

void HandleRemoveCharFromGuild(int UserIndex) {
	/* '*************************************************** */
	/* 'Author: Juan Martín Sotuyo Dodero (Maraxus) */
	/* 'Last Modification: 12/26/06 */
	/* ' */
	/* '*************************************************** */
	if (UserList[UserIndex].sockctx->incomingData->length() < 3) {
		throw insufficient_data_error();
		return;
	}

	/* 'This packet contains strings, make a copy of the data to prevent losses if it's not complete yet... */
	clsByteQueue* buffer = UserList[UserIndex].sockctx->incomingData.get();


	/* 'Remove packet ID */
	buffer->ReadByte();

	std::string UserName;
	int GuildIndex;

	UserName = buffer->ReadUnicodeString();

	if (!UserTienePrivilegio(UserIndex, PlayerType_RoleMaster)
			&& UserTieneAlgunPrivilegios(UserIndex, PlayerType_Admin, PlayerType_Dios)) {

		LogGM(UserList[UserIndex].Name, "/RAJARCLAN " + UserName);

		GuildIndex = m_EcharMiembroDeClan(UserIndex, UserName);

		if (GuildIndex == 0) {
			WriteConsoleMsg(UserIndex, "No pertenece a ningún clan o es fundador.",
					FontTypeNames_FONTTYPE_INFO);
		} else {
			WriteConsoleMsg(UserIndex, "Expulsado.", FontTypeNames_FONTTYPE_INFO);
			SendData(SendTarget_ToGuildMembers, GuildIndex,
					PrepareMessageConsoleMsg(
							UserName + " ha sido expulsado del clan por los administradores del servidor.",
							FontTypeNames_FONTTYPE_GUILD));
		}
	}

	/* 'If we got here then packet is complete, copy data back to original queue */


}

/* '' */
/* ' Handle the "RequestCharMail" message */
/* ' */
/* ' @param userIndex The index of the user sending the message */

void HandleRequestCharMail(int UserIndex) {
	/* '*************************************************** */
	/* 'Author: Juan Martín Sotuyo Dodero (Maraxus) */
	/* 'Last Modification: 12/26/06 */
	/* 'Request user mail */
	/* '*************************************************** */
	if (UserList[UserIndex].sockctx->incomingData->length() < 3) {
		throw insufficient_data_error();
		return;
	}

	/* 'This packet contains strings, make a copy of the data to prevent losses if it's not complete yet... */
	clsByteQueue* buffer = UserList[UserIndex].sockctx->incomingData.get();


	/* 'Remove packet ID */
	buffer->ReadByte();

	std::string UserName;
	std::string mail;

	UserName = buffer->ReadUnicodeString();

	if (UserTieneAlgunPrivilegios(UserIndex, PlayerType_Admin, PlayerType_Dios)
			|| UserList[UserIndex].flags.PrivEspecial) {
		if (FileExist(GetCharPath(UserName))) {
			mail = GetVar(GetCharPath(UserName), "CONTACTO", "email");

			WriteConsoleMsg(UserIndex, "Last email de " + UserName + ":" + mail, FontTypeNames_FONTTYPE_INFO);
		}
	}

	/* 'If we got here then packet is complete, copy data back to original queue */


}

/* '' */
/* ' Handle the "SystemMessage" message */
/* ' */
/* ' @param userIndex The index of the user sending the message */

void HandleSystemMessage(int UserIndex) {
	/* '*************************************************** */
	/* 'Author: Lucas Tavolaro Ortiz (Tavo) */
	/* 'Last Modification: 12/29/06 */
	/* 'Send a message to all the users */
	/* '*************************************************** */
	if (UserList[UserIndex].sockctx->incomingData->length() < 3) {
		throw insufficient_data_error();
		return;
	}

	/* 'This packet contains strings, make a copy of the data to prevent losses if it's not complete yet... */
	clsByteQueue* buffer = UserList[UserIndex].sockctx->incomingData.get();


	/* 'Remove packet ID */
	buffer->ReadByte();

	std::string message;
	message = buffer->ReadUnicodeString();

	if (UserTieneAlgunPrivilegios(UserIndex, PlayerType_Admin, PlayerType_Dios)) {
		LogGM(UserList[UserIndex].Name, "Mensaje de sistema:" + message);

		SendData(SendTarget_ToAll, 0, PrepareMessageShowMessageBox(message));
	}
}

/* '' */
/* ' Handle the "SetMOTD" message */
/* ' */
/* ' @param userIndex The index of the user sending the message */

void HandleSetMOTD(int UserIndex) {
	/* '*************************************************** */
	/* 'Author: Lucas Tavolaro Ortiz (Tavo) */
	/* 'Last Modification: 03/31/07 */
	/* 'Set the MOTD */
	/* 'Modified by: Juan Martín Sotuyo Dodero (Maraxus) */
	/* '   - Fixed a bug that prevented from properly setting the new number of lines. */
	/* '   - Fixed a bug that caused the player to be kicked. */
	/* '*************************************************** */
	if (UserList[UserIndex].sockctx->incomingData->length() < 3) {
		throw insufficient_data_error();
		return;
	}

	/* 'This packet contains strings, make a copy of the data to prevent losses if it's not complete yet... */
	clsByteQueue* buffer = UserList[UserIndex].sockctx->incomingData.get();


	/* 'Remove packet ID */
	buffer->ReadByte();

	std::string newMOTD;
	std::vector<std::string> auxiliaryString;
	int LoopC;

	newMOTD = buffer->ReadUnicodeString();
	auxiliaryString = vb6::Split(newMOTD, vbCrLf);

	if (((!UserTieneAlgunPrivilegios(UserIndex, PlayerType_RoleMaster)
			&& UserTieneAlgunPrivilegios(UserIndex, PlayerType_Admin, PlayerType_Dios)))
			|| UserList[UserIndex].flags.PrivEspecial) {

		LogGM(UserList[UserIndex].Name, "Ha fijado un nuevo MOTD");

		int MaxLines = vb6::UBound(auxiliaryString) + 1;

		MOTD.redim(1, MaxLines);

		WriteVar(GetDatPath(DATPATH::MOTD), "INIT", "NumLines", vb6::CStr(MaxLines));

		for (LoopC = (1); LoopC <= (MaxLines); LoopC++) {
			WriteVar(GetDatPath(DATPATH::MOTD), "Motd", "Line" + vb6::CStr(LoopC),
					auxiliaryString[LoopC - 1]);

			MOTD[LoopC].texto = auxiliaryString[LoopC - 1];
		}

		WriteConsoleMsg(UserIndex, "Se ha cambiado el MOTD con éxito.", FontTypeNames_FONTTYPE_INFO);
	}

	/* 'If we got here then packet is complete, copy data back to original queue */


}

/* '' */
/* ' Handle the "ChangeMOTD" message */
/* ' */
/* ' @param userIndex The index of the user sending the message */

void HandleChangeMOTD(int UserIndex) {
	/* '*************************************************** */
	/* 'Author: Juan Martín sotuyo Dodero (Maraxus) */
	/* 'Last Modification: 12/29/06 */
	/* 'Change the MOTD */
	/* '*************************************************** */
	/* 'Remove Packet ID */
	UserList[UserIndex].sockctx->incomingData->ReadByte();

	if ((!UserTieneAlgunPrivilegios(UserIndex, PlayerType_Dios, PlayerType_Admin))) {
		return;
	}

	std::string auxiliaryString;
	int LoopC;

	for (LoopC = (vb6::LBound(MOTD)); LoopC <= (vb6::UBound(MOTD)); LoopC++) {
		auxiliaryString = auxiliaryString + MOTD[LoopC].texto + vbCrLf;
	}

	if (vb6::Len(auxiliaryString) >= 2) {
		if (vb6::Right(auxiliaryString, 2) == vbCrLf) {
			auxiliaryString = vb6::Left(auxiliaryString, vb6::Len(auxiliaryString) - 2);
		}
	}

	WriteShowMOTDEditionForm(UserIndex, auxiliaryString);
}

/* '' */
/* ' Handle the "Ping" message */
/* ' */
/* ' @param userIndex The index of the user sending the message */

void HandlePing(int UserIndex) {
	/* '*************************************************** */
	/* 'Author: Lucas Tavolaro Ortiz (Tavo) */
	/* 'Last Modification: 12/24/06 */
	/* 'Show guilds messages */
	/* '*************************************************** */
	/* 'Remove Packet ID */
	UserList[UserIndex].sockctx->incomingData->ReadByte();

	WritePong(UserIndex);
}

/* '' */
/* ' Handle the "SetIniVar" message */
/* ' */
/* ' @param userIndex The index of the user sending the message */

void HandleSetIniVar(int UserIndex) {
	/* '*************************************************** */
	/* 'Author: Brian Chaia (BrianPr) */
	/* 'Last Modification: 01/23/10 (Marco) */
	/* 'Modify server.ini */
	/* '*************************************************** */
	if (UserList[UserIndex].sockctx->incomingData->length() < 6) {
		throw insufficient_data_error();
		return;
	}

	/* 'This packet contains strings, make a copy of the data to prevent losses if it's not complete yet... */
	clsByteQueue* buffer = UserList[UserIndex].sockctx->incomingData.get();

	/* 'Remove packet ID */
	buffer->ReadByte();

	std::string sLlave;
	std::string sClave;
	std::string sValor;

	/* 'Obtengo los parámetros */
	sLlave = buffer->ReadUnicodeString();
	sClave = buffer->ReadUnicodeString();
	sValor = buffer->ReadUnicodeString();

	WriteConsoleMsg(UserIndex, "Comando deshabilitado.", FontTypeNames_FONTTYPE_INFO);
	return;
#if 0
	if (UserTieneAlgunPrivilegios(UserIndex, PlayerType_Admin, PlayerType_Dios)) {
		std::string sTmp;

		/* 'No podemos modificar [INIT]Dioses ni [Dioses]* */
		if ((vb6::UCase(sLlave) == "INIT" && vb6::UCase(sClave) == "DIOSES")
				|| vb6::UCase(sLlave) == "DIOSES") {
			WriteConsoleMsg(UserIndex, "¡No puedes modificar esa información desde aquí!",
					FontTypeNames_FONTTYPE_INFO);
		} else {
			/* 'Obtengo el valor según llave y clave */
			sTmp = GetVar(GetIniPath("server.ini"), sLlave, sClave);

			/* 'Si obtengo un valor escribo en el server.ini */
			if (vb6::LenB(sTmp)) {
				WriteVar(GetIniPath("server.ini"), sLlave, sClave, sValor);
				LogGM(UserList[UserIndex].Name,
						"Modificó en server.ini (" + sLlave + " " + sClave + ") el valor " + sTmp + " por "
								+ sValor);
				WriteConsoleMsg(UserIndex,
						"Modificó " + sLlave + " " + sClave + " a " + sValor + ". Valor anterior " + sTmp,
						FontTypeNames_FONTTYPE_INFO);
			} else {
				WriteConsoleMsg(UserIndex, "No existe la llave y/o clave", FontTypeNames_FONTTYPE_INFO);
			}
		}
	}

	/* 'If we got here then packet is complete, copy data back to original queue */
#endif
}

/* '' */
/* ' Handle the "CreatePretorianClan" message */
/* ' */
/* ' @param userIndex The index of the user sending the message */

void HandleCreatePretorianClan(int UserIndex) {
	/* '*************************************************** */
	/* 'Author: ZaMa */
	/* 'Last Modification: 29/10/2010 */
	/* '*************************************************** */

	int Map;
	int X;
	int Y;

	/* 'Remove packet ID */
	UserList[UserIndex].sockctx->incomingData->ReadByte();

	Map = UserList[UserIndex].sockctx->incomingData->ReadInteger();
	X = UserList[UserIndex].sockctx->incomingData->ReadByte();
	Y = UserList[UserIndex].sockctx->incomingData->ReadByte();

	/* ' User Admin? */
	if (!UserTieneAlgunPrivilegios(UserIndex, PlayerType_Admin, PlayerType_Dios)) {
		return;
	}

	/* ' Valid pos? */
	if (!InMapBounds(Map, X, Y)) {
		WriteConsoleMsg(UserIndex, "Posición inválida.", FontTypeNames_FONTTYPE_INFO);
		return;
	}

	WriteConsoleMsg(UserIndex, "Pretorianos deshabilitados.", FontTypeNames_FONTTYPE_INFO);

#if 0
	/* ' Choose pretorian clan index */
	if (Map == MAPA_PRETORIANO) {
		/* ' Default clan */
		index = 1;
	} else {
		/* ' Custom Clan */
		index = 2;
	}

	/* ' Is already active any clan? */
	if (!ClanPretoriano[index].Active) {

		if (!ClanPretoriano[index].SpawnClan(Map, X, Y, index)) {
			WriteConsoleMsg(UserIndex, "La posición no es apropiada para crear el clan", FontTypeNames_FONTTYPE_INFO);
		}

	} else {
		WriteConsoleMsg(UserIndex, "El clan pretoriano se encuentra activo en el mapa " + vb6::CStr(ClanPretoriano[index].ClanMap) + ". Utilice /EliminarPretorianos MAPA y reintente.", FontTypeNames_FONTTYPE_INFO);
	}

	LogGM(UserList[UserIndex].Name, "Utilizó el comando /CREARPRETORIANOS " + vb6::CStr(Map) + " " + vb6::CStr(X) + " " + vb6::CStr(Y));
#endif
}

/* '' */
/* ' Handle the "CreatePretorianClan" message */
/* ' */
/* ' @param userIndex The index of the user sending the message */

void HandleDeletePretorianClan(int UserIndex) {
	/* '*************************************************** */
	/* 'Author: ZaMa */
	/* 'Last Modification: 29/10/2010 */
	/* '*************************************************** */

	int Map;

	/* 'Remove packet ID */
	UserList[UserIndex].sockctx->incomingData->ReadByte();

	Map = UserList[UserIndex].sockctx->incomingData->ReadInteger();

	/* ' User Admin? */
	if (!UserTieneAlgunPrivilegios(UserIndex, PlayerType_Admin, PlayerType_Dios)) {
		return;
	}

	/* ' Valid map? */
	if (Map < 1 || Map > NumMaps) {
		WriteConsoleMsg(UserIndex, "Mapa inválido.", FontTypeNames_FONTTYPE_INFO);
		return;
	}

	WriteConsoleMsg(UserIndex, "Pretorianos deshabilitados.", FontTypeNames_FONTTYPE_INFO);

	/* FIXME: PRETORIANOS */

#if 0
	for (index = (1); index <= (vb6::UBound(ClanPretoriano)); index++) {

		/* ' Search for the clan to be deleted */
		if (ClanPretoriano[index].ClanMap == Map) {
			ClanPretoriano[index].DeleteClan();
			break;
		}

	}

	LogGM(UserList[UserIndex].Name, "Utilizó el comando /ELIMINARPRETORIANOS " + vb6::CStr(Map));
#endif
}

/* '' */
/* ' Handles the "SetDialog" message. */
/* ' */
/* ' @param UserIndex The index of the user sending the message */

void HandleSetDialog(int UserIndex) {
	/* '*************************************************** */
	/* 'Author: Amraphen */
	/* 'Last Modification: 18/11/2010 */
	/* '20/11/2010: ZaMa - Arreglo privilegios. */
	/* '*************************************************** */
	if (UserList[UserIndex].sockctx->incomingData->length() < 2) {
		throw insufficient_data_error();
		return;
	}

	/* 'This packet contains strings, make a copy of the data to prevent losses if it's not complete yet... */
	clsByteQueue* buffer = UserList[UserIndex].sockctx->incomingData.get();


	/* 'Remove packet id */
	buffer->ReadByte();

	std::string NewDialog;
	NewDialog = buffer->ReadUnicodeString();

	if (UserList[UserIndex].flags.TargetNPC > 0) {
		/* ' Dsgm/Dsrm/Rm */
		if (UserTieneAlgunPrivilegios(UserIndex, PlayerType_Admin, PlayerType_Dios, PlayerType_SemiDios)) {
			/* 'Replace the NPC's dialog. */
			Npclist[UserList[UserIndex].flags.TargetNPC].desc = NewDialog;
		}
	}
}

/* '' */
/* ' Handles the "Impersonate" message. */
/* ' */
/* ' @param    userIndex The index of the user sending the message. */

void HandleImpersonate(int UserIndex) {
	/* '*************************************************** */
	/* 'Author: ZaMa */
	/* 'Last Modification: 20/11/2010 */
	/* ' */
	/* '*************************************************** */

	/* 'Remove packet ID */
	UserList[UserIndex].sockctx->incomingData->ReadByte();

	/* ' Dsgm/Dsrm/Rm */
	if (!UserTieneAlgunPrivilegios(UserIndex, PlayerType_Admin, PlayerType_Dios, PlayerType_SemiDios)) {
		return;
	}

	int NpcIndex;
	NpcIndex = UserList[UserIndex].flags.TargetNPC;

	if (NpcIndex == 0) {
		return;
	}

	/* ' Copy head, body and desc */
	ImitateNpc(UserIndex, NpcIndex);

	/* ' Teleports user to npc's coords */
	WarpUserChar(UserIndex, Npclist[NpcIndex].Pos.Map, Npclist[NpcIndex].Pos.X, Npclist[NpcIndex].Pos.Y,
			false, true);

	/* ' Log gm */
	LogGM(UserList[UserIndex].Name,
			"/IMPERSONAR con " + Npclist[NpcIndex].Name + " en mapa " + vb6::CStr(UserList[UserIndex].Pos.Map));

	/* ' Remove npc */
	QuitarNPC(NpcIndex);
}

/* '' */
/* ' Handles the "Imitate" message. */
/* ' */
/* ' @param    userIndex The index of the user sending the message. */

void HandleImitate(int UserIndex) {
	/* '*************************************************** */
	/* 'Author: ZaMa */
	/* 'Last Modification: 20/11/2010 */
	/* ' */
	/* '*************************************************** */

	/* 'Remove packet ID */
	UserList[UserIndex].sockctx->incomingData->ReadByte();

	/* ' Dsgm/Dsrm/Rm/ConseRm */
	if (UserTieneAlgunPrivilegios(UserIndex, PlayerType_Admin, PlayerType_Dios, PlayerType_SemiDios)) {
		return;
	}

	int NpcIndex;
	NpcIndex = UserList[UserIndex].flags.TargetNPC;

	if (NpcIndex == 0) {
		return;
	}

	/* ' Copy head, body and desc */
	ImitateNpc(UserIndex, NpcIndex);
	LogGM(UserList[UserIndex].Name,
			"/MIMETIZAR con " + Npclist[NpcIndex].Name + " en mapa " + vb6::CStr(UserList[UserIndex].Pos.Map));

}

/* '' */
/* ' Handles the "RecordAdd" message. */
/* ' */
/* ' @param UserIndex The index of the user sending the message */

void HandleRecordAdd(int UserIndex) {
	/* '************************************************************** */
	/* 'Author: Amraphen */
	/* 'Last Modify Date: 29/11/2010 */
	/* ' */
	/* '************************************************************** */
	if (UserList[UserIndex].sockctx->incomingData->length() < 2) {
		throw insufficient_data_error();
		return;
	}

	/* 'This packet contains strings, make a copy of the data to prevent losses if it's not complete yet... */
	clsByteQueue* buffer = UserList[UserIndex].sockctx->incomingData.get();


	/* 'Remove packet id */
	buffer->ReadByte();

	std::string UserName;
	std::string Reason;

	UserName = buffer->ReadUnicodeString();
	Reason = buffer->ReadUnicodeString();

	if (!(UserTieneAlgunPrivilegios(UserIndex, PlayerType_User, PlayerType_Consejero, PlayerType_RoleMaster))) {
		/* 'Verificamos que exista el personaje */
		if (!FileExist(GetCharPath(UserName))) {
			WriteShowMessageBox(UserIndex, "El personaje no existe");
		} else {
			/* 'Agregamos el seguimiento */
			AddRecord(UserIndex, UserName, Reason);

			/* 'Enviamos la nueva lista de personajes */
			WriteRecordList(UserIndex);
		}
	}
}

/* '' */
/* ' Handles the "RecordAddObs" message. */
/* ' */
/* ' @param UserIndex The index of the user sending the message. */

void HandleRecordAddObs(int UserIndex) {
	/* '************************************************************** */
	/* 'Author: Amraphen */
	/* 'Last Modify Date: 29/11/2010 */
	/* ' */
	/* '************************************************************** */
	if (UserList[UserIndex].sockctx->incomingData->length() < 3) {
		throw insufficient_data_error();
		return;
	}

	/* 'This packet contains strings, make a copy of the data to prevent losses if it's not complete yet... */
	clsByteQueue* buffer = UserList[UserIndex].sockctx->incomingData.get();


	/* 'Remove packet id */
	buffer->ReadByte();

	int RecordIndex;
	std::string Obs;

	RecordIndex = buffer->ReadByte();
	Obs = buffer->ReadUnicodeString();

	if (!(UserTieneAlgunPrivilegios(UserIndex, PlayerType_User, PlayerType_Consejero, PlayerType_RoleMaster))) {
		/* 'Agregamos la observación */
		AddObs(UserIndex, RecordIndex, Obs);

		/* 'Actualizamos la información */
		WriteRecordDetails(UserIndex, RecordIndex);
	}
}

/* '' */
/* ' Handles the "RecordRemove" message. */
/* ' */
/* ' @param UserIndex The index of the user sending the message. */

void HandleRecordRemove(int UserIndex) {
	/* '*************************************************** */
	/* 'Author: Amraphen */
	/* 'Last Modification: 29/11/2010 */
	/* ' */
	/* '*************************************************** */
	int RecordIndex;

	/* 'Remove packet ID */
	UserList[UserIndex].sockctx->incomingData->ReadByte();

	RecordIndex = UserList[UserIndex].sockctx->incomingData->ReadByte();

	if (UserTieneAlgunPrivilegios(UserIndex, PlayerType_User, PlayerType_Consejero, PlayerType_RoleMaster)) {
		return;
	}

	/* 'Sólo dioses pueden remover los seguimientos, los otros reciben una advertencia: */
	if (UserTieneAlgunPrivilegios(UserIndex, PlayerType_Dios)) {
		RemoveRecord(RecordIndex);
		WriteShowMessageBox(UserIndex, "Se ha eliminado el seguimiento.");
		WriteRecordList(UserIndex);
	} else {
		WriteShowMessageBox(UserIndex, "Sólo los dioses pueden eliminar seguimientos.");
	}
}

/* '' */
/* ' Handles the "RecordListRequest" message. */
/* ' */
/* ' @param UserIndex The index of the user sending the message. */

void HandleRecordListRequest(int UserIndex) {
	/* '*************************************************** */
	/* 'Author: Amraphen */
	/* 'Last Modification: 29/11/2010 */
	/* ' */
	/* '*************************************************** */
	/* 'Remove packet ID */
	UserList[UserIndex].sockctx->incomingData->ReadByte();

	if (UserTieneAlgunPrivilegios(UserIndex, PlayerType_User, PlayerType_Consejero, PlayerType_RoleMaster)) {
		return;
	}

	WriteRecordList(UserIndex);
}

/* '' */
/* ' Handles the "RecordDetailsRequest" message. */
/* ' */
/* ' @param UserIndex The index of the user sending the message. */

void HandleRecordDetailsRequest(int UserIndex) {
	/* '*************************************************** */
	/* 'Author: Amraphen */
	/* 'Last Modification: 07/04/2011 */
	/* 'Handles the "RecordListRequest" message */
	/* '*************************************************** */
	int RecordIndex;

	/* 'Remove packet ID */
	UserList[UserIndex].sockctx->incomingData->ReadByte();

	RecordIndex = UserList[UserIndex].sockctx->incomingData->ReadByte();

	if (UserTieneAlgunPrivilegios(UserIndex, PlayerType_User, PlayerType_Consejero, PlayerType_RoleMaster)) {
		return;
	}

	WriteRecordDetails(UserIndex, RecordIndex);
}

void HandleMoveItem(int UserIndex) {
	/* '*************************************************** */
	/* 'Author: Ignacio Mariano Tirabasso (Budi) */
	/* 'Last Modification: 01/01/2011 */
	/* ' */
	/* '*************************************************** */

	int originalSlot;
	int newSlot;

	UserList[UserIndex].sockctx->incomingData->ReadByte();

	originalSlot = UserList[UserIndex].sockctx->incomingData->ReadByte();
	newSlot = UserList[UserIndex].sockctx->incomingData->ReadByte();
	UserList[UserIndex].sockctx->incomingData->ReadByte();

	moveItem(UserIndex, originalSlot, newSlot);

}

/* '' */
/* ' Handles the "HigherAdminsMessage" message. */
/* ' */
/* ' @param    userIndex The index of the user sending the message. */

void HandleHigherAdminsMessage(int UserIndex) {
	/* '*************************************************** */
	/* 'Author: Torres Patricio (Pato) */
	/* 'Last Modification: 03/30/12 */
	/* ' */
	/* '*************************************************** */
	if (UserList[UserIndex].sockctx->incomingData->length() < 3) {
		throw insufficient_data_error();
		return;
	}

	/* 'This packet contains strings, make a copy of the data to prevent losses if it's not complete yet... */
	clsByteQueue* buffer = UserList[UserIndex].sockctx->incomingData.get();


	/* 'Remove packet ID */
	buffer->ReadByte();

	std::string message;

	message = buffer->ReadUnicodeString();

	if (UserTieneAlgunPrivilegios(UserIndex, PlayerType_Admin, PlayerType_Dios)) {
		LogGM(UserList[UserIndex].Name, "Mensaje a Dioses:" + message);

		if (vb6::LenB(message) != 0) {
			/* 'Analize chat... */
			ParseChat(message);

			SendData(SendTarget_ToHigherAdminsButRMs, 0,
					PrepareMessageConsoleMsg(UserList[UserIndex].Name + "(Sólo Dioses)> " + message,
							FontTypeNames_FONTTYPE_GMMSG));
		}
	}

	/* 'If we got here then packet is complete, copy data back to original queue */


}
/* '' */
/* ' Handle the "AlterName" message */
/* ' */
/* ' @param userIndex The index of the user sending the message */

void HandleAlterGuildName(int UserIndex) {
	/* '*************************************************** */
	/* 'Author: Lex! */
	/* 'Last Modification: 14/05/12 */
	/* 'Change guild name */
	/* '*************************************************** */
	if (UserList[UserIndex].sockctx->incomingData->length() < 5) {
		throw insufficient_data_error();
		return;
	}

	/* 'This packet contains strings, make a copy of the data to prevent losses if it's not complete yet... */
	clsByteQueue* buffer = UserList[UserIndex].sockctx->incomingData.get();


	/* 'Remove packet ID */
	buffer->ReadByte();

	/* 'Reads the userName and newUser Packets */
	std::string GuildName;
	std::string newGuildName;
	int GuildIndex;

	GuildName = buffer->ReadUnicodeString();
	newGuildName = buffer->ReadUnicodeString();
	GuildName = vb6::Trim(GuildName);
	newGuildName = vb6::Trim(newGuildName);

	if (!UserTieneAlgunPrivilegios(UserIndex, PlayerType_RoleMaster) && UserTieneAlgunPrivilegios(UserIndex, PlayerType_Admin, PlayerType_Dios)) {
		if (vb6::LenB(GuildName) == 0 || vb6::LenB(newGuildName) == 0) {
			WriteConsoleMsg(UserIndex, "Usar: /ACLAN origen@destino", FontTypeNames_FONTTYPE_INFO);
		} else {
			/* 'Revisa si el nombre nuevo del clan existe */
			if ((vb6::InStrB(newGuildName, "+") != 0)) {
				GuildName = vb6::Replace(newGuildName, "+", " ");
			}

			GuildIndex = GetGuildIndex(newGuildName);
			if (GuildIndex > 0) {
				WriteConsoleMsg(UserIndex, "El clan destino ya existe.", FontTypeNames_FONTTYPE_INFO);
			} else {
				/* 'Revisa si el nombre del clan existe */
				if ((vb6::InStrB(GuildName, "+") != 0)) {
					GuildName = vb6::Replace(GuildName, "+", " ");
				}

				GuildIndex = GetGuildIndex(GuildName);
				if (GuildIndex > 0) {
					/* ' Existe clan origen y no el de destino */
					/* ' Verifica si existen archivos del clan, los crea con nombre nuevo y borra los viejos */
					if (FileExist(GetGuildsPath(GuildName, EGUILDPATH::Members))) {
						vb6::FileCopy(GetGuildsPath(GuildName, EGUILDPATH::Members),
								GetGuildsPath(newGuildName, EGUILDPATH::Members));
						vb6::Kill(GetGuildsPath(GuildName, EGUILDPATH::Members));
					}

					if (FileExist(GetGuildsPath(GuildName, EGUILDPATH::Relaciones))) {
						vb6::FileCopy(GetGuildsPath(GuildName, EGUILDPATH::Relaciones),
								GetGuildsPath(newGuildName, EGUILDPATH::Relaciones));
						vb6::Kill(GetGuildsPath(GuildName, EGUILDPATH::Relaciones));
					}

					if (FileExist(GetGuildsPath(GuildName, EGUILDPATH::Propositions))) {
						vb6::FileCopy(GetGuildsPath(GuildName, EGUILDPATH::Propositions),
								GetGuildsPath(newGuildName, EGUILDPATH::Propositions));
						vb6::Kill(GetGuildsPath(GuildName, EGUILDPATH::Propositions));
					}

					if (FileExist(GetGuildsPath(GuildName, EGUILDPATH::Solicitudes))) {
						vb6::FileCopy(GetGuildsPath(GuildName, EGUILDPATH::Solicitudes),
								GetGuildsPath(newGuildName, EGUILDPATH::Solicitudes));
						vb6::Kill(GetGuildsPath(GuildName, EGUILDPATH::Solicitudes));
					}

					if (FileExist(GetGuildsPath(GuildName, EGUILDPATH::Votaciones))) {
						vb6::FileCopy(GetGuildsPath(GuildName, EGUILDPATH::Votaciones),
								GetGuildsPath(newGuildName, EGUILDPATH::Votaciones));
						vb6::Kill(GetGuildsPath(GuildName, EGUILDPATH::Votaciones));
					}

					/* ' Actualiza nombre del clan en guildsinfo y server */
					WriteVar(GetDatPath(DATPATH::GuildsInfo), "GUILD" + vb6::CStr(GuildIndex), "GuildName", newGuildName);
					SetNewGuildName(GuildIndex, newGuildName);

					/* ' Actualiza todos los online del clan */
					for (auto MemberIndex : guild_Iterador_ProximoUserIndex(GuildIndex)) {
						if ((UserList[MemberIndex].ConnID != -1)) {
							RefreshCharStatus(MemberIndex);
						}
					}

					/* ' Avisa que sali? todo OK y guarda en log del GM */
					WriteConsoleMsg(UserIndex,
							"El clan " + GuildName + " fue renombrado como " + newGuildName,
							FontTypeNames_FONTTYPE_INFO);
					LogGM(UserList[UserIndex].Name,
							"Ha cambiado el nombre del clan " + GuildName + ". Ahora se llama "
									+ newGuildName);
				} else {
					WriteConsoleMsg(UserIndex, "El clan origen no existe.", FontTypeNames_FONTTYPE_INFO);
				}
			}
		}
	}

	/* 'If we got here then packet is complete, copy data back to original queue */

}
