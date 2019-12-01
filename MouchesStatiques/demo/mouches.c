
/*ster33.c


POPULATION         5000
DECOUPE             200
GENERATIONS          50
MUTATION-0 RATE       .0
MUTATION-1 RATE       .4
CROSSOVER RATE        .2
CROSSOVER RANGE        1
SHARING RADIUS         2
SHARING COEFF          .3
FOCALE              400.
PROXIMUS               40
REMOTUS              700
ZOOM                   2
DISPLAY RATE          .4
COLOURS                3
TOURNAMENT             0
WINDOW SIZE (0 SMALL 1 BIG)  1

-
--------------------------------------
*/

#include <stdlib.h>
#include <math.h>
#include <stdio.h>

#define MAXPOPULATION 50000         /* taille maxi - taille reelle reglee par le fichier setup.txt */
#define LARGEUR_HISTO 1000          /* reglable - en pratique entre 200 et 1000 */

#define WIDTH  1024                     /* TAILLE MAXI - reglee automatiquement par la lecture des fichiers*/
#define HEIGHT 1024                     /* TAILLE MAXI - reglee automatiquement*/
#define RESERV 1*WIDTH*HEIGHT

/********************* ici, la structure "mouche":  ***********************/

typedef struct {float x;float y;float z;float fit;int rank;int flag;int life;}
        mouche;
/**************************************************************************/
int i,j,k,l,m,n,p;
int col;
int width, height;
int ZOOM; /* parametre pour l'affichage de la carte des obstacles */
float DISPLAYRATE;  /* proportion de mouches reellement affichees a la fin */

int POPULATION;
int DECOUPE; /* paramètre pour le tri */
int generation;
int mutants0, mutants1, croisants, survivants;
int evaluations;
float a,b,c,d,e,f,a1,b1,c1,d1,e1,a2,b2,c2,d2,e2,a3,b3,c3,d3,e3,f3,a4,b4,c4,d4,e4,f4;
float moy,truc,machin,bidule;
int xi,eta,zeta,xid,etad;
float R11,R12,R13,R21,R22,R23,R31,R32,R33,TX,TY,TZ; /* parametres de la matrice fondamentale */

/* xi, eta coords dans l'image gauche
   xid, etad coords dans l'image droite
   zeta = decalage sur l'epipolaire (arrondi en entier, ce qui n'est pas ideal).
*/

unsigned char gauche[15+RESERV],droite[15+RESERV],map[RESERV],dist[RESERV],grad[RESERV];
unsigned char param[100];
int igauche[WIDTH][HEIGHT][1], idroite[WIDTH][HEIGHT][1];

float igradient[WIDTH][HEIGHT];
float sharing[WIDTH][HEIGHT];

float idist[WIDTH][HEIGHT];
float imap[2000][2000] ;

/* parametres et variables genetiques */

mouche indiv[MAXPOPULATION];
int first[MAXPOPULATION];

int GENERATIONS;
float MUT0, MUT1;
float CROISEMENTS;
float RANGE;        /* crossover range */
int SHARING_RADIUS;
float SHARING_COEFF;
int TYPE;
int COLOURS;
int TOURNAMENT;
int WINDOWSIZE;
float  maxfitness, minfitness, ecart,meanfitness = 1;

/* parametres geometriques */

float FOCALE;
float PROXIMUS, REMOTUS;

/* fonctions */

void init(void);
void input(void);
void output(void);
void process(void);
void eval(void);
void selection(void);
void mutation(void);
void crossover (void);
void affichage(void);

float randfloat0(void);
float randfloat1(void);

void Fusionner(mouche* , int , int , int ); // Fonctions pour la sélection
void Tri_Fusion(mouche* , int , int );
void gradient (int,int); // pour un calcul par mouche
void visu_gradient(void);

/**********************************************/
void init()
{

for(i=0;i<POPULATION;i++)
  {

indiv[i].z = PROXIMUS/(.0001+randfloat0());

                   /* repartition des profondeurs:
                      z de PROXIMUS a l'infini
                      donc zeta uniforme de presque-zero a -TX*FOCALE/PROXIMUS*/

   indiv[i].x = indiv[i].z * width  * (randfloat0()-.5) / FOCALE;
   indiv[i].y = indiv[i].z * height * (randfloat0()-.5) / FOCALE;


   indiv[i].fit  = 0.;
   indiv[i].rank = i;              /* ranking */
   indiv[i].flag = 0;              /* variable utilitaire pour sort() */
   indiv[i].life = 0;             /* 0 = vivants, pas encore evalues */
  }

for (i=0;i<width;i++)
  {for (j=0;j<height;j++)
     {
      imap[i][j]=0; idist[i][j]=0;
      igradient[i][j]=0;
      sharing[i][j] = 1.; /* important de ne pas mettre zero !!! */
     }
  }

/* Réécriture du gradient dans une fonction ?part et utilisée dans eval() */
/* pour éviter des calculs inutiles */

/* Initialisation de igradient */

 for( i=0 ; i<WIDTH ; i++)
   for (j=0 ; j<HEIGHT ;j++)
     igradient[i][j] = -100000.;



}

/************************************************************************************************/
void selection()
{
int imut, icrois;
int seuil_crois;
int seuil_mut0;
int seuil_mut1;

if (TOURNAMENT == 0)    /* selection par ranking de sous-populations */
  {
   seuil_crois = DECOUPE*CROISEMENTS;
   seuil_mut0 = seuil_crois + DECOUPE*MUT0;
   seuil_mut1 = seuil_mut0 + DECOUPE*MUT1;

   for (i=0 ; i<POPULATION/DECOUPE ; i++)
     {
      j=i*DECOUPE;
      Tri_Fusion(indiv, j, j+DECOUPE-1);
      for(k=j ; k<j+seuil_crois ;k++)             indiv[k].life = -3;
      for(k=j+seuil_crois ; k<j+seuil_mut0 ;k++)  indiv[k].life = -1;
      for(k=j+seuil_mut0 ; k<j+seuil_mut1 ;k++)    indiv[k].life = -2;
     }
  }

else if (TOURNAMENT==1)  /* selection par tournoi*/
  {
   imut=0;
   icrois=0;
   while(imut<POPULATION*MUT0)
     {
      j=POPULATION*randfloat0();
      if(indiv[j].life == 1)
        {
         k=POPULATION*randfloat0();
         if (indiv[k].life == 1)
           {
            imut++;
            if (indiv[j].fit<indiv[k].fit) indiv[j].life=-1;
            else indiv[k].life = -1;
           }
        }
     }
   while(imut<POPULATION*MUT1)
     {
      j=POPULATION*randfloat0();
      if(indiv[j].life == 1)
        {
         k=POPULATION*randfloat0();
         if (indiv[k].life == 1)
           {
            imut++;
            if (indiv[j].fit<indiv[k].fit) indiv[j].life=-2;
            else indiv[k].life = -2;
           }
        }
     }
   while(icrois<POPULATION*CROISEMENTS)
     {
      j=POPULATION*randfloat0();
      if(indiv[j].life == 1)
        {
         k=POPULATION*randfloat0();
         if (indiv[k].life == 1)
           {
            icrois++;
            if (indiv[j].fit<indiv[k].fit) indiv[j].life=-3;
            else indiv[k].life = -3;
           }
        }
     }
  }

meanfitness = 0.;
mutants0= 0; mutants1=0; croisants = 0; survivants = 0;

/*  calcul de la dynamique */

maxfitness=-1000000000.;
for(i=0;i<POPULATION;i++)if(indiv[i].fit > maxfitness) maxfitness = indiv[i].fit;
minfitness = maxfitness;
for(i=0;i<POPULATION;i++)if(indiv[i].fit < minfitness) minfitness = indiv[i].fit;
for(i=0;i<POPULATION;i++)
  {
   if (indiv[i].life == 1) {meanfitness += indiv[i].fit;survivants++;}
   else if (indiv[i].life == -1) mutants0 ++;
   else if (indiv[i].life == -2) mutants1++;
   else if (indiv[i].life == -3) croisants ++;
  }
meanfitness /= survivants;
printf ("max=%7.3f min=%8.3f mean=%7.3f ",maxfitness,minfitness,meanfitness);
printf ("mut0=%d  mut1=%d crois=%d surv=%d",mutants0, mutants1, croisants, survivants );

}

/************************************************************************************************/

void gradient( int xi, int eta) {

float xidinf, etadinf, xidprox, etadprox, u,v;

  if ((xi>=2)&&(xi<width-2)&&(eta>=2)&&(eta<height-2)) {

 /* si l'objet est a l'infini : */
        truc= R31*(xi-width/2.)/FOCALE + R32*(eta-height/2.)/FOCALE + R33;
        xidinf = R11*(xi-width/2.)/FOCALE + R12*(eta-height/2.)/FOCALE + R13;
        xidinf = FOCALE*xidinf/truc + width/2.;
        etadinf = R21*(xi-width/2.)/FOCALE + R22*(eta-height/2.)/FOCALE + R23;
        etadinf = FOCALE*etadinf/truc + height/2.;

 /* si l'objet est tout pres */
        truc= R31*(xi-width/2.)/FOCALE + R32*(eta-height/2.)/FOCALE + R33 + TZ/PROXIMUS;
        xidprox = R11*(xi-width/2.)/FOCALE + R12*(eta-height/2.)/FOCALE + R13 + TX/PROXIMUS;
        xidprox = FOCALE*xidprox/truc + width/2.;
        etadprox = R21*(xi-width/2.)/FOCALE + R22*(eta-height/2.)/FOCALE + R23 + TY/PROXIMUS;
        etadprox = FOCALE*etadprox/truc + height/2.;

        truc = (xidprox-xidinf)*(xidprox-xidinf) + (etadprox-etadinf)*(etadprox-etadinf);

        if (truc<4.) igradient[xi][eta] = 0.;  /* si on est en plein dans le FOE */

        else
          {
           truc = sqrt(truc);

           u=2.*(xidprox-xidinf)/truc;
           v=2.*(etadprox-etadinf)/truc;      /*vecteur unitaire de la direction epipolaire*/

           i= (int)(xidinf + u); j = (int)(xidinf - u);
           k= (int)(etadinf + v); l = (int) (etadinf - v);
           b = 0.;
           if((i>2)&&(j>2)&&(i<width-2)&&(j<width-2))
           if((k>2)&&(l>2)&&(k<height-2)&&(l<height-2))
           for(col=0;col<COLOURS;col++)
             {a = idroite[i][k][col]-idroite[j][l][col];
              a *= a;
              b += a;
              a = idroite[i+1][k][col]-idroite[j+1][l][col];
              a *= a;
              b += a;
              a = idroite[i][k+1][col]-idroite[j][l+1][col];
              a *= a;
              b += a;
              a = idroite[i+1][k+1][col]-idroite[j+1][l+1][col];
              a *= a;
              b += a;
             }
           if(truc<4) b /= 2.;     /* si on est un peu trop pres du FOE */
           igradient[xi][eta]=b;
          }
       }

}

/*******************************************************************************************************************************/

void eval()
{
                                         /* calculs de sharing */

 if (SHARING_RADIUS>0)for(i=0;i<width;i++)for(j=0;j<height;j++) sharing[i][j]=0;
 if (SHARING_RADIUS>0)for(n=0;n<POPULATION;n++)
   {
    xi=(int)(FOCALE*indiv[n].x/indiv[n].z + .5 * width );
    eta=(int)(FOCALE*indiv[n].y/indiv[n].z + .5 * height);
    if((xi<width)&&(xi>=0)&&(eta<height)&&(eta>=0))
      {
       i=(int)(xi  /SHARING_RADIUS);
       j=(int)(eta /SHARING_RADIUS);
       sharing[i][j] += 2. ;
       if(i>0) sharing[i-1][j] += 1;
       if(i+1<width) sharing[i+1][j] += 1;
       if(j>0) sharing[i][j-1] += 1;
       if(j+1<height) sharing[i][j+1] += 1;
      }
   }
                                          /* calculs de fitness */

 for(n=0;n<POPULATION;n++)
   {
    if (indiv[n].life == 0)
      {  /* si a re-evaluer alors j'evalue */
       evaluations ++;
       indiv[n].life = 1;
       indiv[n].fit = 0.;
         /* calculs des coords de projection de la mouche dans l'image de gauche */
       a = indiv[n].x;b=indiv[n].y;c=indiv[n].z;               /* cette boucle doit tourner vite ! */
       xi= (int)((FOCALE * a / c ) + .5 * width );
       eta=(int)((FOCALE * b / c ) + .5 * height);

         /* calculs des coords de projection de la mouche dans l'image de droite */
      /* cas general */
        {
         d = R31*a + R32*b + R33*c + TZ ;
         if(d > .001)
           {
            xid=(int)((FOCALE * ((R11*a) + (R12*b) + (R13*c) + TX) / d) + (.5 * width ) );
            etad=(int)((FOCALE * ((R21*a) + (R22*b) + (R23*c) + TY) / d) + (.5 * height) );
           }
        }
           /* fin des calculs des coords de projection dans les cameras */

      if ((xi+6<=width)&&(xi>=5)&&(eta+3<=height)&&(eta>=2)
           &&(xid+6<=width)&&(xid>=5)&&(etad+3<=height)&&(etad>=2))
        {
	  /* Calcul du gradient seulement o?on en a besoin */
	  if (igradient[xid][etad] == -100000) {
	    gradient(xid, etad);
	  }

         if (WINDOWSIZE==1)   /* 1ere option, moins de faux appariements mais calcul plus lourd */
           {
            truc=0.;
            for(col=0;col<COLOURS;col++)
              {
               a =igauche[xi-3][eta]  [col]-idroite[xid-3][etad]  [col];
               b =igauche[xi-1][eta]  [col]-idroite[xid-1][etad]  [col];
               c =igauche[xi]  [eta]  [col]-idroite[xid]  [etad]  [col];
               d =igauche[xi+1][eta]  [col]-idroite[xid+1][etad]  [col];
               e =igauche[xi+3][eta]  [col]-idroite[xid+3][etad]  [col];
               a1=igauche[xi-2][eta-1][col]-idroite[xid-2][etad-1][col];
               b1=igauche[xi-1][eta-1][col]-idroite[xid-1][etad-1][col];
               c1=igauche[xi]  [eta-1][col]-idroite[xid]  [etad-1][col];
               d1=igauche[xi+1][eta-1][col]-idroite[xid+1][etad-1][col];
               e1=igauche[xi+2][eta-1][col]-idroite[xid+2][etad-1][col];
               a2=igauche[xi-2][eta+1][col]-idroite[xid-2][etad+1][col];
               b2=igauche[xi-1][eta+1][col]-idroite[xid-1][etad+1][col];
               c2=igauche[xi]  [eta+1][col]-idroite[xid]  [etad+1][col];
               d2=igauche[xi+1][eta+1][col]-idroite[xid+1][etad+1][col];
               e2=igauche[xi+2][eta+1][col]-idroite[xid+2][etad+1][col];
/*modif depuis version 29 */
               a3=igauche[xi-3][eta-3][col]-idroite[xid-3][etad-3][col];
               b3=igauche[xi-1][eta-2][col]-idroite[xid-1][etad-2][col];
               c3=igauche[xi]  [eta-3][col]-idroite[xid]  [etad-3][col];
               d3=igauche[xi+1][eta-2][col]-idroite[xid+1][etad-2][col];
               e3=igauche[xi+3][eta-3][col]-idroite[xid+3][etad-3][col];
               a4=igauche[xi-3][eta+3][col]-idroite[xid-3][etad+3][col];
               b4=igauche[xi-1][eta+2][col]-idroite[xid-1][etad+2][col];
               c4=igauche[xi]  [eta+3][col]-idroite[xid]  [etad+3][col];
               d4=igauche[xi+1][eta+2][col]-idroite[xid+1][etad+2][col];
               e4=igauche[xi+3][eta+3][col]-idroite[xid+3][etad+3][col];

               moy=(a+b+c+d+e+a1+b1+c1+d1+e1+a2+b2+c2+d2+e2+a3+b3+c3+d3+e3+a4+b4+c4+d4+e4)/25.;
               a -= moy;  b -= moy;  c -= moy;  d -= moy; e -= moy;
               a1 -= moy; b1 -= moy; c1 -= moy; d1 -= moy; e1 -= moy;
               a2 -= moy; b2 -= moy; c2 -= moy; d2 -= moy; e2 -= moy;
               a3 -= moy; b3 -= moy; c3 -= moy; d3 -= moy; e3 -= moy;
               a4 -= moy; b4 -= moy; c4 -= moy; d4 -= moy; e4 -= moy;

               truc += a*a+b*b+c*c+d*d+e*e+a1*a1+b1*b1+c1*c1+d1*d1+e1*e1
                +a2*a2+b2*b2+c2*c2+d2*d2+e2*e2+a3*a3+b3*b3+c3*c3+d3*d3+e3*e3+a4*a4+b4*b4+c4*c4+d4*d4+e4*e4;
              }
            truc = 1.+1000*sqrt(igradient[xid][etad])/(1.+truc);
            indiv[n].fit = log(truc)  ;
            if(indiv[n].fit < -100000.)  indiv[i].fit=-100000.;
           }
         else if (WINDOWSIZE==0)
           /* debut 2eme option: fenetre de comparaison + petite - on gagne en temps
              d'execution au prix de petites ambiguites */
           {
            truc=0.; machin = 0.; bidule = 0.;
            for(col=0;col<COLOURS;col++)
              {
               a=igauche[xi-2][eta][col]-idroite[xid-2][etad][col];
               b=igauche[xi-1][eta][col]-idroite[xid-1][etad][col];
               c=igauche[xi]  [eta][col]-idroite[xid]  [etad][col];
               d=igauche[xi+1][eta][col]-idroite[xid+1][etad][col];
               e=igauche[xi+2][eta][col]-idroite[xid+2][etad][col];
               a1=igauche[xi-2][eta-1][col]-idroite[xid-2][etad-1][col];
               b1=igauche[xi-1][eta-1][col]-idroite[xid-1][etad-1][col];
               c1=igauche[xi]  [eta-1][col]-idroite[xid]  [etad-1][col];
               d1=igauche[xi+1][eta-1][col]-idroite[xid+1][etad-1][col];
               e1=igauche[xi+2][eta-1][col]-idroite[xid+2][etad-1][col];
               a2=igauche[xi-2][eta+1][col]-idroite[xid-2][etad+1][col];
               b2=igauche[xi-1][eta+1][col]-idroite[xid-1][etad+1][col];
               c2=igauche[xi]  [eta+1][col]-idroite[xid]  [etad+1][col];
               d2=igauche[xi+1][eta+1][col]-idroite[xid+1][etad+1][col];
               e2=igauche[xi+2][eta+1][col]-idroite[xid+2][etad+1][col];
               moy=(a+b+c+d+e+a1+b1+c1+d1+e1+a2+b2+c2+d2+e2)/15.;
               a -= moy;  b -= moy;  c -= moy;  d -= moy; e -= moy;
               a1 -= moy; b1 -= moy; c1 -= moy; d1 -= moy; e1 -= moy;
               a2 -= moy; b2 -= moy; c2 -= moy; d2 -= moy; e2 -= moy;
               truc += a*a+b*b+c*c+d*d+e*e+a1*a1+b1*b1+c1*c1+d1*d1+e1*e1
                +a2*a2+b2*b2+c2*c2+d2*d2+e2*e2;
              }
            truc = 1.+1000*sqrt(igradient[xid][etad])/(1.+truc);
            indiv[n].fit = log(truc)  ;
            if(indiv[n].fit < -100000.)  indiv[i].fit=-100000.;
           }
             /* fin de la 2eme option */
        }
         /* fin de la condition sur le champ de vision */

      else indiv[n].fit = 0.;  /* hors du champ de vision commun des 2 cameras */
      if ( (SHARING_RADIUS > 0) &&
          ( (int)(xi/SHARING_RADIUS)  < width ) &&
          ( (int)(eta/SHARING_RADIUS) < height) &&
          ( xi  >=0 ) &&
          ( eta >=0 ))
        {
	  indiv[n].fit -= SHARING_COEFF*sharing[(int)(xi/SHARING_RADIUS)][(int)(eta/SHARING_RADIUS)];

         if(indiv[n].fit < -100000.)
           {
            indiv[i].fit=-100000.; printf(" fitness trop basse apres sharing\n ");
           }
        }
     }
  }
}

/************************************/
void mutation0()
{
 for (i=0; i<POPULATION; i++)
 if (indiv[i].life == -1)   /*life = -1 : mort, a re-creer par mutation sans pere */
   { /* comme ?l'initialisation */
    indiv[i].life = 0;

    indiv[i].z = PROXIMUS/(.0001+randfloat0());

    indiv[i].x = indiv[i].z * width  * (randfloat0()-.5) / FOCALE;
    indiv[i].y = indiv[i].z * height * (randfloat0()-.5) / FOCALE;

  }
}
/************************************/
void mutation1()
{
 for (i=0; i<POPULATION; i++)
 if (indiv[i].life == -2)   /*life = -2 : mort, a re-creer par mutation avec pere */
  {
   j = (int)((float)POPULATION*randfloat0()); /* un pere choisi au hasard dans l'ancienne population */
   c =indiv[j].z;
   if ((c > PROXIMUS)&&(c < REMOTUS))

    {
     /* attention, ne pas expliciter xi, eta et zeta qui sont entiers ! */

     indiv[i].x = c * ((FOCALE * indiv[j].x / c)+ 10. * randfloat1() ) / FOCALE;
     indiv[i].y = c * ((FOCALE * indiv[j].y / c)+ 10. * randfloat1() ) / FOCALE;
     indiv[i].z = PROXIMUS/((PROXIMUS/c)+.04*randfloat1());    /* modif 2.05.2000 */


     indiv[i].life = 0;  /* life = 0 : vivant, pas encore evalue */
    }
   else
     indiv[i].life = 1;
  }
}
/************************************/
void crossover()
{
float lambda,mu, denom;
for (i=0; i<POPULATION; i++)
 if (indiv[i].life == -3)   /*life = -3 : mort, a re-creer par croisement */
   {
    j = (int)((float)POPULATION*randfloat0()); /* un parent */
    k = (int)((float)POPULATION*randfloat0()); /* l'autre parent */

lambda = RANGE * (-.5 + randfloat0());
mu=1.-lambda;

a=indiv[j].x;b=indiv[k].x;
c=indiv[j].y;d=indiv[k].y;
e=indiv[j].z;f=indiv[k].z;

indiv[i].x=lambda*a+mu*b;
indiv[i].y=lambda*c+mu*d;
if(lambda*e+mu*f>.00001)indiv[i].z=lambda*e+mu*f;

indiv[i].life = 0;  /* life = 0 : vivant, pas encore evalue */
   }
}

/************************************/
void affichage()
{
c=(-2000.*REMOTUS)/(PROXIMUS-REMOTUS);
a=2000./(PROXIMUS-REMOTUS);


for(i=0;i<2000;i++)   for(j=0;j<2000;j++)     imap[i][j] = 255;

for(n=0;n<POPULATION;n++) if(indiv[n].life == 1) /* si vivant ET evalue */
  {
   i=(int)(width*.5  + indiv[n].x*FOCALE/indiv[n].z);
   j=(int)(height*.5 + indiv[n].y*FOCALE/indiv[n].z);
   k=(int)((5.-FOCALE*TX/indiv[n].z)*ZOOM);

   if ( (i>=0) && (i<width) && (j>=0) && (j<height) && (k>=0) && (k<height) )
     {
      idist[i-1][j-1] = k;
      idist[i-1][j] = k;
      idist[i-1][j+1] = k;
      idist[i][j-1] = k;
      idist[i][j] = k;
      idist[i][j+1] = k;
      idist[i+1][j-1] = k;
      idist[i+1][j] = k;
      idist[i+1][j+1] = k;
     }

   i=(int)(-a*indiv[n].x + 2000);
   j=(int)(a*indiv[n].z + c);

   if((i>=0)&&(i<2000)&&(j>=0)&&(j<2000)) imap[i][j]=0;

 /* if(n<DECOUPE) printf("\nn=%d\ti=%d\tj=%d",n,i,j);*/

  }




/* inversion des couleurs */
   for(i=0;i<width;i++) for(j=0;j<height;j++)  idist[i][j] = 255 - idist[i][j];


/* graduations */

   for(i=0;10*i<2000;i++) {imap[10*i][5*ZOOM+1] = 127;imap[10*i][5*ZOOM+2] = 127;} /* graduation horizontale */
   for(i=0;i<2000;i++) imap[i][5*ZOOM] = 127;                             /* axe horizontal */


}
/**********************************************************/
void input()
{
int truc;
FILE *fp;

  fp = fopen("gauche.pbm","rb");
  fscanf(fp,"P%d\n",&truc); /* truc = 4, 5 ou 6 */
  fscanf(fp,"%d %d\n",&width,&height);
  fclose (fp);

  if(width*height > WIDTH*HEIGHT) {printf("format image trop grand\n");exit(1);}
  printf("width = %d  height = %d \n", width, height);

  fp = fopen("gauche.pbm","rb");
  fread (gauche,1,RESERV+15,fp);
  fclose(fp);

  fp = fopen("droite.pbm","rb");
  fread (droite,1,RESERV+15,fp);
  fclose(fp);

/* lecture des parametres et codage en entier */

 for (i=0;i<width;i++)
   {for (j=0;j<height;j++)
      {for (col=0;col<1;col++)
         {igauche[i][j][col] = (int) gauche[15+1*(i+width*j)+col];
          idroite[i][j][col] = (int) droite[15+1*(i+width*j)+col];
         }
      }
   }
/* lecture des parametres externes */

if(fp = fopen("setup.txt","rb"))
 {
  fscanf(fp,"POPULATION %d\n", &POPULATION);
  fscanf(fp,"DECOUPE %d\n", &DECOUPE);
  fscanf(fp,"GENERATIONS %d\n",&GENERATIONS);
  fscanf(fp,"MUTATION-0 RATE %f\n",&MUT0);
  fscanf(fp,"MUTATION-1 RATE %f\n",&MUT1);
  fscanf(fp,"CROSSOVER RATE %f\n",&CROISEMENTS);
  fscanf(fp,"CROSSOVER RANGE %f\n",&RANGE);
  fscanf(fp,"SHARING RADIUS %d\n",&SHARING_RADIUS);
  fscanf(fp,"SHARING COEFF %f\n",&SHARING_COEFF);
  fscanf(fp,"FOCALE %f\n",&FOCALE);
  fscanf(fp,"PROXIMUS %f\n",&PROXIMUS);
  fscanf(fp,"REMOTUS %f\n",&REMOTUS);
  fscanf(fp,"ZOOM %d\n",&ZOOM);
  fscanf(fp,"DISPLAY RATE %f\n",&DISPLAYRATE);
  fscanf(fp,"COLOURS %d\n",&COLOURS);
  fscanf(fp,"TOURNAMENT %d\n",&TOURNAMENT);
  fscanf(fp,"WINDOW SIZE (0 SMALL 1 BIG)  %d",&WINDOWSIZE);

  fclose(fp);
  if (MUT0+MUT1+CROISEMENTS >= .8) {printf("taux mutations + croisement excessif\n");exit(1);}

printf ("POPULATION %d\n", POPULATION);
printf ("DECOUPE %d\n", DECOUPE);
printf ("GENERATIONS = %d\n",GENERATIONS);
printf ("MUTATIONS-0 = %f  MUTATIONS-1 = %f\nCROISEMENTS = %f\n",MUT0,MUT1,CROISEMENTS);
printf ("CROSSOVER RANGE = %f\n",RANGE);
printf ("SHARING RADIUS = %d\n",SHARING_RADIUS);
printf ("SHARING COEFF = %f\n",SHARING_COEFF);
printf ("Distance FOCALE = %f\n",FOCALE);
printf ("Punctus proximus = %f\n",PROXIMUS);
printf ("Punctus remotus = %f\n",REMOTUS);
if (REMOTUS <= PROXIMUS) {printf ("valeurs incorrectes\n"); exit(0);}
printf ("zoom affichage = %d\n",ZOOM);
printf ("display rate = %f\n",DISPLAYRATE);
printf ("number of colour channels used: %d\n",COLOURS);
printf ("TOURNAMENT = %d",TOURNAMENT);
  if (TOURNAMENT==1) printf("   selection avec tournoi");
  else if (TOURNAMENT==0) printf ("   selection par ranking de sous-populations");
printf ("\nwindow size = %d",WINDOWSIZE);
  if (WINDOWSIZE==0) printf("  petite fenetre de correlation");
  else if (WINDOWSIZE==1) printf ("   grande fenetre de correlation");
if ((ZOOM <= 0)||(ZOOM > 100)) {printf("\nZOOM doit etre entier, compris entre 1 et 100\n");exit(1);}

 }
else {printf("fichier setup.txt non trouve\n");exit(1);}


 /* lecture de la matrice fondamentale "fondam.txt" */
if(fp = fopen("fondam.txt","rb"))
 {
  fscanf(fp,"%f;", &R11);
  fscanf(fp,"%f;", &R12);
  fscanf(fp,"%f;", &R13);
  fscanf(fp,"%f\n;", &TX);
  fscanf(fp,"%f;", &R21);
  fscanf(fp,"%f;", &R22);
  fscanf(fp,"%f;", &R23);
  fscanf(fp,"%f\n;", &TY);
  fscanf(fp,"%f;", &R31);
  fscanf(fp,"%f;", &R32);
  fscanf(fp,"%f;", &R33);
  fscanf(fp,"%f;", &TZ);
  fclose(fp);
 }
else {printf("fondam.txt file not found in current directory\n");exit(1);}
printf("\ncoefficients matrice fondamentale:\n");
printf ("%5.3f\t%5.3f\t%5.3f\t%5.3f\n",R11,R12,R13,TX);
printf ("%5.3f\t%5.3f\t%5.3f\t%5.3f\n",R21,R22,R23,TY);
printf ("%5.3f\t%5.3f\t%5.3f\t%5.3f\n",R31,R32,R33,TZ);

}
/*******************************************/
void output()
{
int i,j;
FILE *fp;

for (i=0;i<width;i++)
  {for (j=0;j<height;j++)
     {

      dist[1*(i+width*j)] = (unsigned char) (idist[i][j]);
      //dist[1+1*(i+width*j)] = (unsigned char) (idist[i][j]);
      //dist[2+1*(i+width*j)] = (unsigned char) (idist[i][j]);

      grad[1*(i+width*j)] = (unsigned char) (igradient[i][j]);
      //grad[1+1*(i+width*j)] = (unsigned char) (igradient[i][j]);
      //grad[2+1*(i+width*j)] = (unsigned char) (igradient[i][j]);

     }
  }
for (i=0;i<2000;i++)for(j=0;j<2000;j++)
  {
   map[1*(i+2000*j)] = (unsigned char) (imap[i][j]);
   //map[1+1*(i+600*j)] = (unsigned char) (imap[i][j]);
   //map[2+1*(i+600*j)] = (unsigned char) (imap[i][j]);
  }
fp=fopen("map3.pbm","wb");
fprintf(fp,"P5\n%d %d\n255\n",2000,2000);
fwrite(map,1,1*2000*2000,fp);
fclose(fp);

fp=fopen("dist3.pbm","wb");
fprintf(fp,"P5\n%d %d\n255\n",width,height);
fwrite(dist,1,1*width*height,fp);
fclose(fp);

fp=fopen("grad3.pbm","wb");
fprintf(fp,"P5\n%d %d\n255\n",width,height);
fwrite(grad,1,1*width*height,fp);
fclose(fp);


}


/******************************************* FUSIONNER */

void Fusionner(mouche *A, int p, int q, int r)

{
  mouche B[r+1];
  int i,j,k;


  for(i=p;i<=q;i++){
    B[i].x = A[i].x;
    B[i].y = A[i].y;
    B[i].z = A[i].z;
    B[i].fit = A[i].fit;
    B[i].rank = A[i].rank;
    B[i].flag = A[i].flag;
    B[i].life = A[i].life;
  }

  for(j=q+1;j<=r;j++){
    B[r+q+1-j].x = A[j].x;
    B[r+q+1-j].y = A[j].y;
    B[r+q+1-j].z = A[j].z;
    B[r+q+1-j].fit = A[j].fit;
    B[r+q+1-j].rank = A[j].rank;
    B[r+q+1-j].flag = A[j].flag;
    B[r+q+1-j].life = A[j].life;
  }
  i=p;
  j=r;
  for(k=p;k<=r;k++)
    {

      if (B[i].fit<B[j].fit)
        {

	  A[k].x = B[i].x;
	  A[k].y = B[i].y;
	  A[k].z = B[i].z;
	  A[k].fit = B[i].fit;
	  A[k].rank = B[i].rank;
	  A[k].flag = B[i].flag;
	  A[k].life = B[i].life;

          i=i+1;
        }
      else
        {
	  A[k].x = B[j].x;
	  A[k].y = B[j].y;
	  A[k].z = B[j].z;
	  A[k].fit = B[j].fit;
	  A[k].rank = B[j].rank;
	  A[k].flag = B[j].flag;
	  A[k].life = B[j].life;
          j=j-1;
        }
    }
}

/****************************************** TRI_FUSION */


void Tri_Fusion(mouche *A, int p, int r)

{
  int q;

  if (p<r)
    {
      q=(p+r)/2;
      Tri_Fusion(A,p,q);
      Tri_Fusion(A,q+1,r);
      Fusionner(A,p,q,r);
    }
}

/************************************/
float randfloat0(void)
{
  float alea, numerator, denominator;

  numerator = rand() & 0x7fff;
  denominator = 0x7fff;

  alea = numerator/denominator;
  return alea;  /* equireparti sur [0,1] */
}
/**************************************************************** RANDFLOAT1 */
float randfloat1(void)
{
  float alea, numerator, denominator;

  numerator = rand() & 0x7fff;
  denominator = 0x7fff;

  alea = 2. *(numerator/denominator) - 1.;
  alea = alea * (.2 + .8 * alea * alea ) ;
  return alea;  /* reparti sur [-1,1], concentre pres de 0 */
}

/*******************************************/

void process()
{


                                     /* optimisation genetique */
generation = 0;
evaluations = 0;
eval();

for(generation=1; generation<GENERATIONS; generation++)
  {

   printf("\ngen.%d\t", generation);

   selection();   /* je marque life = -1 ceux a muter et life = -2 ceux a croiser */
   mutation0();   /* je modifie les life = -1 et je leur donne life = 0: a re-evaluer */
   mutation1();
   crossover();   /* je modifie les life = -3 et je leur donne life = 0 */
   eval();        /* j'evalue ceux dont life = 0 et je leur donne life = 1 */

  }

printf ("\nPOPULATION %d\n", POPULATION);
printf ("DECOUPE %d\n", DECOUPE);
printf ("GENERATIONS = %d\n",GENERATIONS);
printf ("MUTATIONS-0 = %f  MUTATIONS-1 = %f\nCROISEMENTS = %f\n",MUT0,MUT1,CROISEMENTS);
printf ("CROSSOVER RANGE = %f\n",RANGE);
printf ("SHARING RADIUS = %d\n",SHARING_RADIUS);
printf ("SHARING COEFF = %f\n",SHARING_COEFF);
printf ("Distance FOCALE = %f\n",FOCALE);
printf ("Punctus proximus = %f\n",PROXIMUS);
printf ("Punctus remotus = %f\n",REMOTUS);
printf ("zoom affichage = %d\n",ZOOM);
printf ("display rate = %f\n",DISPLAYRATE);
printf ("number of colour channels used: %d\n",COLOURS);
printf ("TOURNAMENT = %d",TOURNAMENT);
  if (TOURNAMENT==1) printf("   selection avec tournoi\n");
  else if (TOURNAMENT==0) printf ("   selection par ranking de sous-populations\n");
printf ("\nwindow size = %d",WINDOWSIZE);
  if (WINDOWSIZE==0) printf("  petite fenetre de correlation\n");
  else if (WINDOWSIZE==1) printf ("   grande fenetre de correlation\n");

printf("nombre total d evaluations %d\n",evaluations);

MUT0=0;
MUT1 = 1. - DISPLAYRATE;
CROISEMENTS = 0.;
TOURNAMENT = 0;
selection();
affichage();
visu_gradient();
printf("\n");

}
//////////////////////////////

void visu_gradient() {

  float min=100000,max=-100000;


   for(i=0;i<width;i++) for(j=0;j<height;j++)
     if (igradient[i][j] != -100000){
       if(igradient[i][j]<min)
	 min = igradient[i][j];
       if (igradient[i][j]>max)
	 max = igradient[i][j];
     }


   printf("min gradient = %f\n",min);
   printf("max gradient = %f\n",max);


   for(i=0;i<width;i++) for(j=0;j<height;j++) /* inversion des couleurs */
  {
    if (igradient[i][j] != -100000)
      igradient[i][j] = 255 - (igradient[i][j]-min)*255/(max-min);
    else
      igradient[i][j] = 255;
  }

}
/************************************************/

main()
{
input();
init();
process();
output();
}
