#ifndef DEATHMATCH_H
#define DEATHMATCH_H
#include <vector>
#include <memory>
#include <string>
#include "vb6compat.h"
#include "enums.h"

extern const int TIEMPO_CANCELAMIENTO;
extern const int BANQUERO;
extern const int MAXCUPOS;
extern const int MINCUPOS;
 
struct tUsuario{
    int id;
    WorldPos posicion;
};
 
struct tDeathmatch{
    bool activo;
    vb6::array<struct tUsuario> usuarios;
    bool objetos;
    int conteo;
    int cupos;
    WorldPos coordenadas;
    int premio;
    int inscripcion;
    int total;
    int restantes;
    WorldPos posBanquero;
    bool esperaPelear;
    bool torneoFinalizado;
    bool esperandoParticipantes;
};
 
void CargaDeath();

void ArmarDeath(int id, int cupos, bool objetos, int premio, int inscripcion);

void EntrarDeath(int id);

int DeathID();
 
bool PuedeEntrar(int id);
 
void ContarDeath();

int IDUsuario();
 
void MuereDeath(int id);
 
void RestarUsuario(int id);
 
void Finalizar();
 
void CancelarDeath();

void DesconexionDeath(int ID);
 
void Limpiar();
 
void AsignarFlagEspera();

#endif
