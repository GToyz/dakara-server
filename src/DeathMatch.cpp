#include "stdafx.h"
#include "DeathMatch.h"

const int TIEMPO_CANCELAMIENTO = 180;
const int BANQUERO = 24;
const int MAXCUPOS = 40;
const int MINCUPOS = 2;

static struct tDeathmatch deathmatch;

void CargaDeath(){ // insetar lugares correspondientes
  deathmatch.coordenadas.Map = 1;
  deathmatch.coordenadas.X = 50;
  deathmatch.coordenadas.Y = 50;
}

void ArmarDeath(int id, int cupos, bool objetos, int premio, int inscripcion){
  if (deathmatch.activo){
	  WriteConsoleMsg(id, "Deathmatch> El evento ya está en curso.", FontTypeNames_FONTTYPE_INFO);
    return;
  }
  if (cupos > MAXCUPOS) cupos = MAXCUPOS;
  if (cupos < MINCUPOS) cupos = MINCUPOS;
  deathmatch.cupos = cupos;
  deathmatch.objetos = objetos;
  deathmatch.premio = premio;
  deathmatch.inscripcion = inscripcion;
  deathmatch.total = cupos;
  deathmatch.activo = true;
  deathmatch.conteo = TIEMPO_CANCELAMIENTO;
  deathmatch.esperandoParticipantes = true;
  deathmatch.usuarios.redim(1, deathmatch.cupos);
  SendData(SendTarget_ToAll, 0,
				dakara::protocol::server::BuildConsoleMsg(
						vb6::CStr("Deathmatch> " + deathmatch.cupos + " Cupos, 
                      Incripción" & vb6::IIf(deathmatch.inscripcion > 0, 
                      " de: " + deathmatch.inscripcion + " Monedas de oro, ",
                      " Gratis, ") + vb6::IIf(deathmatch.objetos, 
                      "Caen items, ", "No caen items, ") + vb6::IIf(deathmatch.premio > 0,
                      "Premio de: " + deathmatch.premio + " Monedas de oro.",
                      " No hay premio.") + " Manden/DEATHMATCH si desean participar.",
                      FontTypeNames_FONTTYPE_INFO));
}
