/* SFML libraries */
#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>

/* Misc libs */
#include <iostream>
#include <string>
#include <iomanip>
#include <locale>
#include <sstream>

/* Calcul de cases par ligne */
#include <math.h>
#include <cmath>

/* change cmd color */
#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <dos.h>
#include <dir.h>

#define DEGTORAD(D)((D * PI) / 180.0) // Converts Degrees to radians
#define RADTODEG(R)((180.0 * R) / PI)//Converts Radians to Degrees

/*
  Formule pour dormir en c++
  sf::Time t2 = sf::milliseconds(3000);
  sleep(t2);
*/

/* Feature request: ajustement des éléments de la fenêtre quand on la redimensionne, ou bloquer cette option */

// Structure case pour gestion tableau
struct boite
{
    int number;                  // pour l'interaction des cases avec les pions
    int event;                   // détermine évènement (serpent/echelle)
    sf::RectangleShape zone;     // rectangle de la case
    sf::Text cellno;             // numéro de la case
    struct boite *prev;          // case antérieure
};

// Structure relative aux pions et joueurs
struct pions
{
    sf::RectangleShape pion;     // rectangle en attendant le sprite
    sf::Text nick_text;          // structure du texte
    struct pions *next;          // déterminer qui est le prochain joueur
    std::string nickname;        // nom du joueur
    int nocase = 1;              // associer le pion à une case (au lieu de pointeurs)
    int no_joueur;               // associer le no joueur au pion correspondant
    int posx;                    // position en X du pion (pour faciliter le traitement)
    int posy;                    // position en Y du pion (pour faciliter le traitement)
};

struct snakes
{
    int positions [4];
    struct snakes *next = NULL;
};

struct ladders
{
    int positions [4];
    struct snakes *next = NULL;
};

// Élément SFML pour les serpents et échelles (temporaire)

// définir la fenêtre
sf::RenderWindow gamewin(sf::VideoMode(840,840),"Snake & Ladder - Version 0.1 (going beta [but still alpha!!!]!)");

// Structures de boite
struct boite *cell;              // structure pour dessiner les cases
struct boite *cell_ant = NULL;   // pour organizer les cases correctement
struct boite *box_end = NULL;    // début du tableau pour fins d'association
struct boite *tmp_cell = NULL;   // structure de manipulation

// Structures de pions
struct pions *joueurs = NULL;    // structure pour créer les joueurs
struct pions *TMP = NULL;        // structure temporaire pour la création des joueurs
struct pions *TMP_next = NULL;   // structure temporaire pour finaliser la file en file circulaire
struct pions *player_on = NULL;  // joueur actuel qui doit jouer son tour

// Structures boosts
struct snakes *serpents = NULL;
struct ladders *echelle = NULL;
struct snakes *tmp_snake = NULL;

int tours = 1;                   // compteur de tours, c'est pour l'affichage du score à la fin
int cases = 36;                  // nombre de cases à dessiner
int PI = 3.14159265;             // pour calculs
bool first_show = true;

sf::Font font;                   // police pour le programme

// fonctions prédéfinies
int main();
void events();
void display();
void restart();
void next_player();
void loadFont();
void make_boosts();

using namespace std;

/* ------------------------- Section relative aux couleurs ----------------------------- */

void SetColor(int ForgC)
{
    WORD wColor;

    HANDLE hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_SCREEN_BUFFER_INFO csbi;

    //We use csbi for the wAttributes word.
    if(GetConsoleScreenBufferInfo(hStdOut, &csbi))
    {
        //Mask out all but the background attribute, and add in the forgournd color
        wColor = (csbi.wAttributes & 0xF0) + (ForgC & 0x0F);
        SetConsoleTextAttribute(hStdOut, wColor);
    }
    return;
}

/* ------------------------- Section relative aux joueurs ----------------------------- */

// combien de joueurs vont jouer
int get_players()
{
    int players;
    cout << "Entrez le nombre de joueurs pour cette partie (2-4) ";
    cin >> players;

    // limites de la version alpha: 4 joueurs max
    if ( players <= 1 || players > 4 )
    {
        // redemander si numéro est hors limites
        cout << "Quantite de joueurs non autorises!" << endl;
        players = get_players();
    }
    return players;
}

// créer les structures des joueurs
void make_players()
{
    // charger nombre de joueurs
    int players = get_players();

    TMP = new(struct pions);
    TMP_next = new(struct pions);

    for ( int cpt = players; cpt >= 1; cpt -- )
    {
        TMP = joueurs;
        joueurs = new(struct pions);
        joueurs->no_joueur = cpt;

        // définir propriétés du pion !!(prochaine version aura des sprite)
        joueurs->pion.setSize(sf::Vector2f(50, 50));
        joueurs->next = TMP;

        // changer les couleurs des pions (while not sprite)
        switch( cpt )
        {
        case 1:
            joueurs->pion.setFillColor(sf::Color::Green);
            joueurs->posx = 10;
            joueurs->posy = 10;
            break;
        case 2:
            joueurs->pion.setFillColor(sf::Color::Yellow);
            joueurs->posx = 70;
            joueurs->posy = 10;
            break;
        case 3:
            joueurs->pion.setFillColor(sf::Color::Magenta);
            joueurs->posx = 10;
            joueurs->posy = 70;
            break;
        case 4:
            joueurs->pion.setFillColor(sf::Color::Cyan);
            joueurs->posx = 70;
            joueurs->posy = 70;
            break;
        }

        SetColor(cpt * 2);

        // lire le nickname
        string name;
        cout << "Joueur " << cpt << ", entrez votre surnom : ";
        cin >> name;

        joueurs->nick_text.setFont(font);
        joueurs->nick_text.setString(name);
        joueurs->nick_text.setCharacterSize(15);
        joueurs->nick_text.setColor(sf::Color::Black);

        joueurs->nickname = name;
    }

    SetColor(7);

    // le joueur qui jouera en premier
    player_on = new(struct pions);
    player_on = joueurs;
}

// dessiner les éléments sur la fenêtre SFML
void draw_players()
{
    TMP = new(struct pions);
    TMP = joueurs;

    while ( joueurs != NULL )
    {
        joueurs->pion.setPosition(joueurs->posx, joueurs->posy);
        joueurs->nick_text.setPosition(joueurs->posx, joueurs->posy);
        gamewin.draw(joueurs->pion);
        gamewin.draw(joueurs->nick_text);

        // changer au prochain joueur
        joueurs = joueurs->next;
    }

    // retourner au joueur original
    joueurs = TMP;

    // dessiner le pion
    gamewin.draw(joueurs->pion);
    gamewin.draw(joueurs->nick_text);
}

/* ------------------------- Section relative au tableau ----------------------------- */

// créer. configurer et dessiner le tableau
void make_table()
{
    // calcul du nombre de cases par ligne
    int cells_line = sqrt(cases);

    // pour le placement des cases (obtenir position antérieure)
    cell_ant = new(struct boite);

    // variables de position
    int count_line = 0;
    int pos_y = 0;
    int pos_x = 0;
    int incr = 0;
    int mod = 0;

    // Changer le fond en rouge (désactivé)
    // gamewin.clear(sf::Color::Red);

    // création, configuration et traçage des cases
    for ( int cnt = 1; cnt <= cases; cnt ++ )
    {
        // créer la nouvelle cellule et assigner taille
        cell = new(struct boite);
        cell->zone.setSize(sf::Vector2f(140, 140));

        // pour vérification ultérieure
        if ( cnt == cases )
        {
            box_end = new(struct boite);
            box_end = cell;
        }

        // pour changer de ligne à temps
        count_line ++;

        // Changer de ligne
        if ( count_line == cells_line + 1 )
        {
            // variables de position et harmonization
            pos_y = pos_y + 140;
            count_line = 1;
            // pos_x = 0;

            // changer l'ordre de ligne pour le motif en damier
            if ( mod == 1 )
            {
                mod = 0;
            }
            else
            {
                mod = 1;
            }
        }
        else // si pas EOL, dessiner en se basant sur les cellules antérieures
        {
            // lignes gauche->droite
            if ( mod == 0 )
            {
                pos_x = cell_ant->zone.getPosition().x + incr;
            }
            // lignes gauche<-droite
            else
            {
                pos_x = cell_ant->zone.getPosition().x - incr;
            }
        }

        // changer couleur de la case pour l'effet damier

        if ( cnt%2 > 0 )
        {
            cell->zone.setFillColor(sf::Color::Blue);    // cellule bleue
        }
        else
        {
            cell->zone.setFillColor(sf::Color::Red);    // cellule rouge
        }

        // assigner le numéro de case à la structure
        cell->number = cnt;

        // Conversion Int à String
        ostringstream num;
        num << cnt;
        string res;
        res = num.str();

        // le numéro de la case en SFML
        cell->cellno.setFont(font);
        cell->cellno.setString(res);
        cell->cellno.setCharacterSize(20);
        cell->cellno.setColor(sf::Color::White);

        // positionner la cellule et le numéro
        cell->zone.setPosition(pos_x,pos_y);
        cell->cellno.setPosition(pos_x, pos_y);

        // Info pour la console (désactivé)
        // cout << "Cell drawn: X:" << pos_x << " Y:" << pos_y << endl;

        // enregistrer la case précédente
        cell->prev = cell_ant;

        // actualiser la case précédente
        cell_ant = cell;

        // truc pour décaler les cellules après le changement de ligne
        incr = 140;

        // dessiner les éléments SFML sur la fenêtre
        gamewin.draw(cell->zone);
        gamewin.draw(cell->cellno);
    }
}

int assigner(int length)
{
    // générer une case
    int no_case = (rand() % length + 5);

    return no_case;
}

int calculer(int no_case)
{
    int nombre = (rand() % (no_case - 5) + 5);
    return nombre;
}

// assigner des serpents ou échelles
void make_boosts()
{
    // quantité de boosts selon le nombre de cases
    int quanta = sqrt(cases);

    // longueur max des boosts ( ~75% )
    int length = cases - 3;

    // structures pour assignation
    struct boite *cnt_cell = NULL;
    struct boite *tmp_contain = NULL;

    if ( first_show == true )
    {

        for ( int cue = 1; cue <= quanta; cue ++ )
        {
                if ( cue > 1)
                {
                                cout << serpents->next << endl;
                                cout << "MYBAD"<<endl;
                }

            // créer structure temporaire
            cnt_cell = box_end; // pas de boost en derniere case

            // calcul des cases & longueurs (nouvelle fonction)
            int no_case = assigner(length);
            int nombre = calculer(no_case);

            while ( cnt_cell->number != no_case )
            {
                cnt_cell = cnt_cell->prev;
            }

            SetColor(3);
            cout << cnt_cell->number;
            SetColor(7);

            cnt_cell->event = nombre;

            // calculer les positions pour le traçage et assigner valeurs à la ligne
            tmp_contain = cnt_cell;

            cnt_cell = box_end;

            cout << "->";
            SetColor(4);
            cout << (no_case - nombre) << endl;
            SetColor(7);

            while ( cnt_cell->number > (no_case - nombre) )
            {
                cnt_cell = cnt_cell->prev;
            }

            // obtenir positions
            int struct_x1 = tmp_contain->zone.getPosition().x;
            int struct_x2 = cnt_cell->zone.getPosition().x;

            int struct_y1 = tmp_contain->zone.getPosition().y;
            int struct_y2 = cnt_cell->zone.getPosition().y;

            if ( tmp_snake != NULL )
            {
                serpents->next = tmp_snake;
            }

            serpents = new(struct snakes);
            serpents->positions[0] = struct_x1;
            serpents->positions[1] = struct_x2;
            serpents->positions[2] = struct_y1;
            serpents->positions[3] = struct_y2;

//        serpents->booster.setFillColor(sf::Color::Black);
//        serpents->booster.setPosition(struct_x1 + 70, struct_y1 + 70);
//        serpents->booster.setSize(sf::Vector2f(15, longueur));
//        serpents->booster.rotate(angle);

            tmp_snake = serpents;

            sf::ConvexShape ConvexShape(4);
            ConvexShape.setOutlineColor(sf::Color::Green); // Couleur
            ConvexShape.setFillColor(sf::Color::Green);
            ConvexShape.setOutlineThickness(3); // Epaisseur

            ConvexShape.setPoint(0, sf::Vector2f(struct_x1 + 68, struct_y1 + 68));
            ConvexShape.setPoint(1, sf::Vector2f(struct_x1 + 72, struct_y1 + 72));
            ConvexShape.setPoint(2, sf::Vector2f(struct_x2 + 72, struct_y2 + 72));
            ConvexShape.setPoint(3, sf::Vector2f(struct_x2 + 68, struct_y2 + 68));

            gamewin.draw(ConvexShape);
        }

        // seulement redessinner en cas de recommencer
        first_show = false;
    }

    else
    {
        tmp_snake = serpents;
        cout << serpents->next << endl;

        // la liste est vide, vérifier pourquoi

        /*
        Créer la liste dans une autre procédure et la remplir ici, apparemment
        c'est pour ça que la liste se vide après qu'elle sorte du for
        */

        while ( tmp_snake != NULL )
        {
                int x1 = serpents->positions[0];
                int x2 = serpents->positions[1];
                int y1 = serpents->positions[2];
                int y2 = serpents->positions[3];

                sf::ConvexShape ConvexShape(4);
                ConvexShape.setOutlineColor(sf::Color::Black); // Couleur
                ConvexShape.setFillColor(sf::Color::Black);
                ConvexShape.setOutlineThickness(3); // Epaisseur

                ConvexShape.setPoint(0, sf::Vector2f(x1 + 68, y1 + 68));
                ConvexShape.setPoint(1, sf::Vector2f(x1 + 72, y1 + 72));
                ConvexShape.setPoint(2, sf::Vector2f(x2 + 72, y2 + 72));
                ConvexShape.setPoint(3, sf::Vector2f(x2 + 68, y2 + 68));

                tmp_snake = tmp_snake->next;
        }
    }
}

/* ------------------------- Section relative au gameplay ----------------------------- */

// générer un nombre au hazard
int dice()
{
    // si possible, une fonction plus "hazardeuse"
    srand(time(NULL));
    int nombre = (rand() % (6 - 1 + 1)) + 1;

    // changer texte
    SetColor(2);
    cout << " got " << nombre << endl;
    SetColor(7);

    return nombre;
}

// fonction gérant le gameplay
void make_turn()
{
    // obtenir le nombre random
    int des = dice();

    // C++ ne veut pas comparer autrement
    int next_cell = player_on->nocase + des;

    // définir les variables locales de position
    int tmpx;
    int tmpy;

    // définir si le jeu est fini
    if ( next_cell == cases )
    {
        SetColor(2);
        // cout << "Player " << player_on->no_joueur << " gagne, apres " << tours << " rondes!" << endl;
        cout << player_on->nickname << " gagne, apres " << tours << " rondes!" << endl;
        gamewin.close();
        return;
    }
    // ne rien faire si le no est plus grand que ce qui est possible
    else if ( next_cell > cases )
    {
        cout << player_on->nickname << " got more than he needs!" << endl;
        // continuer avec le prochain
        next_player();
        return;
    }

    // configurer la position du pion
    tmp_cell = new(struct boite);
    tmp_cell = box_end;

    // trouver la case correspondante
    while ( tmp_cell->number != next_cell )
    {
        // on peut obtenir la case départ et fabriquer une animation!
        tmp_cell = tmp_cell->prev;
    }

    // obtenir la position de la case
    tmpx = tmp_cell->zone.getPosition().x;
    tmpy = tmp_cell->zone.getPosition().y;

    // régler les positions d'après le numéro de joueur
    switch( player_on->no_joueur )
    {
    case 1:
        tmpx = tmpx + 10;
        tmpy = tmpy + 10;
        break;
    case 2:
        tmpx = tmpx + 70;
        tmpy = tmpy + 10;
        break;
    case 3:
        tmpx = tmpx + 10;
        tmpy = tmpy + 70;
        break;
    case 4:
        tmpx = tmpx + 70;
        tmpy = tmpy + 70;
        break;
    }

    // assigner la nouvelle valeur
    player_on->nocase = next_cell;

    // définir la position du pion
    player_on->pion.setPosition(tmpx, tmpy);

    // notifier la nouvelle position
    cout << "Deplace sur case " << player_on->nocase << endl;

    // assigner les nouvelles valeurs de position
    player_on->posx = tmpx;
    player_on->posy = tmpy;

    // changer de joueur
    next_player();

    // afficher le no de joueur actif
    cout << player_on->nickname << "'s turn";

    // rafraîchir la fenêtre et champ de jeu
    display();
}

// cycler à travers les joueurs
void next_player()
{
    // changer de joueur et recommencer si on arrive au dernier joueur
    if ( player_on->next != NULL )
    {
        player_on = player_on->next;
    }
    else
    {
        player_on = joueurs;
        tours ++;
    }
}

// fonction d'affichage
void display()
{
    // dessiner le tableau de jeu
    make_table();

    // organiser les serpents et échelles
    make_boosts();

    // dessiner le(s) pions sur le tableau
    draw_players();

    // afficher les formes
    gamewin.display();
}

/* ------------------------- Section relative à la gestion ----------------------------- */

int main()
{
    // cacher la fenêtre (très sommaire)
    gamewin.setVisible(false);

    // Charger une police
    loadFont();

    // compter et créer joueurs
    make_players();

    // créer et dessnier les éléments SFML
    gamewin.setVisible(true);
    display();

    // message de succès sur la console
    cout << "Board traced successfully" << endl;

    // afficher le no de joueur actif
    //cout << "Player " << player_on->no_joueur << "'s turn";
    cout << player_on->nickname << "'s turn";

    // Évènements pendant que la fenêtre est ouverte (lancer tour, etc)
    while(gamewin.isOpen())
    {
        // lire les évènements fenêtres
        events();
    }

    /* À faire:
       - redéfinir les éléments ( implique d'avoir une fonction reset et passer toutes les structures
                                  en paramètre dans toutes les fonctions [peut-être régler window?] )
                                  ** à noter que seul les joueurs ont besoin d'un reset **
    */
    // relancer le jeu (en construction)
    restart();

    return 0;
}

void loadFont()
{
    // Charger une police pour l'affichage de numéros
    if (!font.loadFromFile("C:/Windows/Fonts/arial.ttf"))
    {
        cout << "font not found" << endl;
    }
}

void restart()
{
    SetColor(7);

    string ans;
    cout << endl << "Voulez-vous rejouer? (y/n) ";
    cin >> ans;

    if ( ans == "y" )
    {
        sf::RenderWindow gamewin(sf::VideoMode(840,840),"Snake & Ladder - Version 0.05 (still pretty much alpha!)");
        main();
    }
}

void events()
{
    sf::Event event;
    while(gamewin.pollEvent(event))
    {
        switch(event.type)
        {
        // La fenêtre se ferme
        case sf::Event::Closed:
            gamewin.close();
            break;

        // Une touche est appuyée
        case sf::Event::KeyPressed:

            // identifier la touche appuyée
            switch(event.key.code)
            {

            // La touche 'espace' es appuyée
            case sf::Keyboard::Space:
                // jouer le tour
                make_turn();
                break;
            }

        // traitement des autres évènemenrs SFML
        default:
            // cout<<"event happenned"<<event.type<<endl;
            break;
        }
    }
}
