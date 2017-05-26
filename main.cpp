/* SFML libraries */
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
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

/*
  Formule pour dormir en c++
  sf::Time t2 = sf::milliseconds(3000);
  sleep(t2);
*/

// Structure case pour gestion tableau
struct boite
{
    int number;                  // pour l'interaction des cases avec les pions
    double event = 0;            // détermine évènement (serpent/echelle)
    sf::RectangleShape zone;     // rectangle de la case dessiné par SFML
    sf::Text cellno;             // numéro de la case
    struct boite *prev;          // case antérieure (pour navigation)
};

// Structure relative aux joueurs et aux pions dessinés
struct pions
{
    /*  sf::RectangleShape pion;     // rectangle en attendant le sprite (un cercle fait plus joli)  */
    sf::CircleShape pion;        // cercle pour faire plus esthétique
    sf::Text nick_text;          // surnom du joueur qui s'affiche à l'écran
    struct pions *next;          // détermine qui est le prochain joueur
    std::string nickname;        // nom du joueur (sauvegardé pour affichage console)
    int nocase = 1;              // associe le pion à une case (au lieu de pointeurs)
    int no_joueur;               // enregistrer le no du joueur
    int posx;                    // position en X du pion
    int posy;                    // position en Y du pion
};

// structures pour le traitement des serpents et échelles
struct boosters
{
    int positions [4];
    struct boosters *next = NULL;
};

// Élément SFML inclus
sf::Font font;                   // police pour le programme
sf::Text message;                // texte pour le jeu (message)
// fenêtre pour le jeu
sf::RenderWindow gamewin(sf::VideoMode(840,840),"Snake & Ladder - Version 0.9 (RC 2)");

// Structures de boite
struct boite *cell;              // structure pour dessiner les cases
struct boite *cell_ant = NULL;   // pour organizer les cases correctement
struct boite *box_end = NULL;    // début du tableau pour fins d'association
struct boite *tmp_cell = NULL;   // structure de manipulation

// Structures de pions
struct pions *joueurs = NULL;    // structure pour créer les joueurs
struct pions *TMP = NULL;        // structure temporaire pour la création des joueurs
struct pions *player_on = NULL;  // joueur actuel qui doit jouer son tour

// structures d'association pour faire la liste des joueurs
struct pions *le_dernier = NULL;
struct pions *le_premier = NULL;
struct pions *le_prochain = NULL;

// Structures boosts
struct boosters *serpents = NULL;
struct boosters *echelle = NULL;
struct boosters *snake_end = NULL;
struct boosters *ladder_end = NULL;

// variables globales
int tours = 0;                   // compteur de tours, c'est pour l'affichage du score à la fin
int cases;                       // nombre de cases à dessiner
int taille = 140;                // taille des cases
int players_global;              // nombre de joueurs dans la partie
bool first_show = true;          // pour construire listes 1 fois seulement

// fonctions prédéfinies (sinon erreur de compilation)
int main();
void events();
void display(bool show_text);
void restart();
void next_player();
void loadFont();
void make_boosts();

using namespace std;

/* ------------------------- Fonction qui change la couleur de la console ----------------------------- */

void SetColor(int ForgC)
{
    /* --liste des couleurs supportées--
    - 0: noir               5: rose foncé           10: vert            15: blanc
    - 1: bleu marin         6: jaune foncé          11: cyan
    - 2: vert foncé         7: gris clair (default) 12: rouge
    - 3: bleu-vert          8: gris foncé           13: rose
    - 4: rouge foncé        9: bleu                 14: jaune
    */

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

int get_players()
{

    cout << "Entrez le nombre de joueurs pour cette partie (2-4) ";
    cin >> players_global;

    switch (players_global)
    {
    case 2:
        break;
    case 3:
        break;
    case 4:
        break;
    default:
        // redemander si numéro est hors limites
        SetColor(4);
        cout << "Quantite de joueurs non autorises!" << endl;
        SetColor(7);

        // redéterminer le nombre de joueurs
        players_global = get_players();

        break;
    }
    return players_global;
}

void calc_niveau()
{
    int level;
    cout << "Choisissez le niveau de difficulte (1/2/3): ";
    cin >> level;

    switch(level)
    {
    case 1:
        // une aire de 4x4
        cases = 16;

        break;
    case 2:
        // une aire de 5x5
        cases = 25;

        break;
    case 3:
        // une aire de 6x6
        cases = 36;

        break;
    default:
        cout << "Hors limites!" << endl;
        calc_niveau();
        break;
    }
    taille = (840/sqrt(cases));
}

void make_loop()
{
    // combien de personnes vont jouer
    players_global = get_players();

    // réinitialiser les structures de position
    le_premier = new(struct pions);
    le_prochain = new(struct pions);
    le_dernier = new(struct pions);

    // clearer les possibles valeurs
    le_premier = NULL;
    le_prochain = NULL;
    le_dernier = NULL;

    for (int cpt = 1; cpt <= players_global; cpt ++)
    {
        joueurs = new(struct pions);

        // pour revenir après la reconstruction
        if (cpt == 1)
        {
            le_premier = joueurs;
        }

        joueurs->next = le_prochain;
        le_prochain = joueurs;
    }

    // retenir pour fermer la liste après
    le_dernier = joueurs;

    // reconstruire la liste
    while (joueurs->next != NULL)
    {
        joueurs=joueurs->next;
    }

    // fermer la boucle
    joueurs->next = le_dernier;

    // retourner au début
    joueurs = joueurs->next;
}

void make_players()
{
    // construire la liste de joueurs avant
    make_loop();

    // couleurs des pions
    sf::Color bleu(44, 44, 255, 180);
    sf::Color jaune(255, 255, 44, 180);
    sf::Color rouge(255, 44, 44, 180);
    sf::Color vert(44, 255, 44, 180);
    sf::Color fg_color(0, 0, 0, 250);

    // experimnental
    for (int counter = 1; counter <= players_global; counter ++)
    {
        // configurer le cercle du pion
        joueurs->pion.setRadius(25);
        joueurs->pion.setPointCount(500);

        // changer les couleurs des pions
        switch( counter )
        {
        case 1:
            joueurs->pion.setFillColor(vert);

            // associer des valeurs en XY pour caser les pions sans juxtaposition
            joueurs->posx = 10;
            joueurs->posy = 10;

            SetColor(10);
            break;
        case 2:
            joueurs->pion.setFillColor(jaune);
            joueurs->posx = 70;
            joueurs->posy = 10;

            SetColor(14);
            break;
        case 3:
            joueurs->pion.setFillColor(rouge);
            joueurs->posx = 10;
            joueurs->posy = 70;

            SetColor(12);
            break;
        case 4:
            joueurs->pion.setFillColor(bleu);
            joueurs->posx = 70;
            joueurs->posy = 70;

            SetColor(9);
            break;
        }

        // lire le nickname
        string name;
        cout << "Joueur " << counter << ", entrez votre surnom : ";
        cin >> name;

        // configurer le label nickname
        joueurs->nick_text.setFont(font);
        joueurs->nick_text.setString(name);
        joueurs->nick_text.setCharacterSize(15);
        joueurs->nick_text.setColor(fg_color);

        // assigner le nick à la structure joueur correspondante
        joueurs->nickname = name;

        // assigner le no de joueur
        joueurs->no_joueur = counter;

        // configurer le prochain joueur
        joueurs=joueurs->next;
    }

    SetColor(7);

    // le joueur qui jouera en premier
    player_on = new(struct pions);
    player_on = joueurs;
}

void draw_players()
{
    // ça ne marche peut-être pas
    // structure temporaire pour le retour
//    TMP = new(struct pions);
//    TMP = joueurs; // le 1er joueur

    for (int cpt = 1; cpt <= players_global; cpt ++)
    {
        joueurs->pion.setPosition(joueurs->posx, joueurs->posy);
        joueurs->nick_text.setPosition(joueurs->posx, joueurs->posy);
        gamewin.draw(joueurs->pion);
        gamewin.draw(joueurs->nick_text);

        // changer au prochain joueur
        joueurs = joueurs->next;
    }

    // retourner au 1er joueur
    joueurs = joueurs->next;

    // dessiner le pion restant

    /* amélioration: cpt = 0 et on évite ce workaround */

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

    // couleurs de cases
    sf::Color bleu(56, 117, 194, 180);
    sf::Color rouge(210, 41, 44, 180);

    // création, configuration et traçage des cases
    for ( int cnt = 1; cnt <= cases; cnt ++ )
    {
        // créer la nouvelle cellule et assigner taille
        cell = new(struct boite);
        cell->zone.setSize(sf::Vector2f(taille, taille));

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
            // variables de position et harmonisation
            pos_y = pos_y + taille;
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
            // cell->zone.setFillColor(sf::Color::Blue);    // cellule bleue
            cell->zone.setFillColor(bleu);                  // cellule bleue
        }
        else
        {
            // cell->zone.setFillColor(sf::Color::Red);    // cellule rouge
            cell->zone.setFillColor(rouge);                // cellule rouge
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

        // enregistrer la case précédente
        cell->prev = cell_ant;

        // actualiser la case précédente
        cell_ant = cell;

        // truc pour décaler les cellules après le changement de ligne
        incr = taille;

        // dessiner les éléments SFML sur la fenêtre
        gamewin.draw(cell->zone);
        gamewin.draw(cell->cellno);
    }
}

/* ----------------------------------- Boosters -----------------------------*/

int create_boosters(int &case_debut, int &case_fin, double &nombre, int cpt, bool type)
{
    // serpents d'abord
    int serpent_debut[] = {8, 11, 21, 34, 18, 25, 12, 30, 20, 5, 28, 22, 15, 9, 35};
    int serpent_fin[] = {4, 2, 17, 12, 17, 14, 10, 23, 13, 2, 22, 16, 6, 3, 25};

    // échelles
    int echelle_debut[] = {3, 6, 10, 5, 27, 16, 19, 4, 10, 14, 31, 29, 25, 8, 13, 10};
    int echelle_fin[] = {7, 15, 20, 24, 29, 27, 34, 22, 11, 33, 35, 30, 36, 24, 17, 25};

    if ( type == true )
    {
        case_debut = serpent_debut[cpt];
        case_fin = serpent_fin[cpt];

        nombre = (case_debut - case_fin) * (-1);
    }
    else
    {
        case_debut = echelle_debut[cpt];
        case_fin = echelle_fin[cpt];

        nombre = case_fin - case_debut;
    }
}
void build_structures(int quanta, struct boosters *structure, struct boosters *end_struct, sf::Color fill_color, sf::Texture fillimage, bool type)
{
    // structures pour naviguer le tableau
    struct boite *cnt_cell = NULL;
    struct boite *tmp_contain = NULL;

    // structures d'opération & association virtuelle
    struct boosters *new_struct = structure;

    // variables de position
    int struct_x1;
    int struct_x2;
    int struct_y1;
    int struct_y2;

    // arrays pour vérifier la non-duplicité des serpents & échelles
    int case_depart[quanta];
    int case_fin[quanta];

    // assignation des valeurs
    for ( int cue = 1; cue <= quanta; cue ++ )
    {

        // créer structure temporaire
        cnt_cell = box_end;

        // puiser les cases depuis les listes
        int case_debut;
        int case_fin;
        double nombre;

        create_boosters(case_debut, case_fin, nombre, cue - 1, type);

        // naviguer jusqu'à trouver la bonne case
        while ( cnt_cell->number != case_debut )
        {
            cnt_cell = cnt_cell->prev;
        }

        // assigner valeur booster
        if ( type == true )
        {
            cnt_cell->event = nombre;
        }
        else
        {
            cnt_cell->event = abs(nombre);
        }

        // calculer les positions pour le traçage et assigner valeurs à la ligne
        tmp_contain = cnt_cell;

        // commencer à naviguer depuis la fin des cases
        cnt_cell = box_end;

        // assigner valeur à cellule (à faire)
        while ( cnt_cell->number > case_fin)
        {
            cnt_cell = cnt_cell->prev;
        }

        // obtenir positions
        struct_x1 = tmp_contain->zone.getPosition().x;
        struct_x2 = cnt_cell->zone.getPosition().x;
        struct_y1 = tmp_contain->zone.getPosition().y;
        struct_y2 = cnt_cell->zone.getPosition().y;

        // sauvegarde des positions
        structure->positions[0] = struct_x1;
        structure->positions[1] = struct_x2;
        structure->positions[2] = struct_y1;
        structure->positions[3] = struct_y2;

        // configuration de la forme
        sf::ConvexShape ConvexShape(4);
        ConvexShape.setOutlineColor(fill_color); // Couleur
        ConvexShape.setFillColor(fill_color);
        ConvexShape.setOutlineThickness(3); // Epaisseur

        // ConvexShape.setTexture(fillimage);

        // passer au prochain bloc
        structure = structure->next;

        // application des positions pour traçage ultérieur
        ConvexShape.setPoint(0, sf::Vector2f(struct_x1 + 68, struct_y1 + 68));
        ConvexShape.setPoint(1, sf::Vector2f(struct_x1 + 72, struct_y1 + 72));
        ConvexShape.setPoint(2, sf::Vector2f(struct_x2 + 68, struct_y2 + 68));
        ConvexShape.setPoint(3, sf::Vector2f(struct_x2 + 72, struct_y2 + 72));
    }

    // seulement redessinner en cas de recommencer
    first_show = false;

    // reset la liste
    structure = end_struct;
}

void draw_boosts(struct boosters *structure, sf::Color fill_color)
{
    // dessinner chaque élément de la liste ?
    while ( structure != NULL )
    {
        int x1 = structure->positions[0];
        int x2 = structure->positions[1];
        int y1 = structure->positions[2];
        int y2 = structure->positions[3];

        sf::ConvexShape ConvexShape(4);
        ConvexShape.setOutlineColor(fill_color); // Couleur
        ConvexShape.setFillColor(fill_color);
        ConvexShape.setOutlineThickness(3); // Epaisseur

        ConvexShape.setPoint(0, sf::Vector2f(x1 + 68, y1 + 68));
        ConvexShape.setPoint(1, sf::Vector2f(x1 + 72, y1 + 72));
        ConvexShape.setPoint(2, sf::Vector2f(x2 + 72, y2 + 72));
        ConvexShape.setPoint(3, sf::Vector2f(x2 + 68, y2 + 68));

        gamewin.draw(ConvexShape);

        structure = structure->next;
    }
}

void make_lists(int quanta)
{
    int cpt = 1;

    struct boosters *contain = NULL;
    contain = NULL;

    // serpents
    while ( cpt <= quanta )
    {
        serpents = new(struct boosters);

        if ( contain != NULL )
        {
            serpents->next = contain;
        }

        contain = serpents;
        cpt ++;
    }

    snake_end = serpents;

    // reset
    cpt = 1;
    contain = NULL;

    // échelles
    while ( cpt <= quanta )
    {
        echelle = new(struct boosters);

        if ( contain != NULL )
        {
            echelle->next = contain;
        }

        contain = echelle;
        cpt ++;
    }

    ladder_end = echelle;
}

void make_boosts()
{
    // quantité de boosts selon le nombre de cases
    int quanta = (sqrt(cases) / 2) + 1;

    if ( cases == 16 )
    {
        // erreur dans la fonction d'incrémenter
        quanta = 2;
    }

    // couleurs pour les formes booster
    sf::Color snake_color(255, 0, 255, 180);
    sf::Color ladder_color(0, 255, 0, 180);

    // sprites pour les choses
    sf::Texture snake_map;
//    if (!snake_map.loadFromFile("serpent.jpg"))
//    {
//            cout << "Failed to load serpent bitmap" << endl;
//    }

    sf::Texture ladder_map;
//    if (!ladder_map.loadFromFile("echelle.png"))
//    {
//            cout << "Failed to load ladder bitmap" << endl;
//    }

    if ( first_show == true )
    {
        // créer la liste de pointeurs
        make_lists(quanta);
    }

    // ré-assignation des valeurs (make_table vient tout clearer avant)

    /* BUG! n'effectue pas la commande */
    build_structures(quanta, serpents, snake_end, snake_color, snake_map, true);
    build_structures(quanta, echelle, ladder_end, ladder_color, ladder_map, false);

    // redessinner les choses déja faites
    draw_boosts(serpents, snake_color);
    draw_boosts(echelle, ladder_color);
}

/* ---------------------- Affiche message ------------------------*/
void popup(int x, int y, string popup_text, int font_size, sf::Color fore_color)
{
    // afficher message reçu
    message.setFont(font);
    message.setString(popup_text);
    message.setCharacterSize(font_size);
    message.setColor(fore_color);

    // positionner la cellule et le numéro
    message.setPosition(x,y);

    // affiche le message
    display(true);
    Sleep(1000);
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
void make_turn(int aller_a)
{
    // obtenir le nombre random
    int des = dice();
    int booster;

    // pour faciliter la transition case/serpent
    int case_bak = 0;

    // sons
    sf::SoundBuffer buf_de;
    sf::SoundBuffer buf_avance;
    sf::SoundBuffer buf_slide;
    sf::SoundBuffer buf_snake;

    // charger fichiers
    if (!buf_de.loadFromFile("dice.wav")) cout << "Import failed" << endl;
    if (!buf_avance.loadFromFile("roar.wav")) cout << "Import failed" << endl;
    if (!buf_slide.loadFromFile("slide.wav")) cout << "Import failed" << endl;
    if (!buf_snake.loadFromFile("snake.wav")) cout << "Import failed" << endl;

    // définition des sons
    sf::Sound son_de;
    sf::Sound son_avance;
    sf::Sound son_slide;
    sf::Sound son_snake;

    // associer au buffer correspondant
    son_de.setBuffer(buf_de);
    son_avance.setBuffer(buf_avance);
    son_slide.setBuffer(buf_slide);
    son_snake.setBuffer(buf_snake);

    // jouer le son
    son_de.play();

    // C++ ne veut pas comparer autrement
    int next_cell = player_on->nocase + des;

    // pour le cheat!
    if ( aller_a != NULL )
    {
        des = aller_a;
        next_cell = aller_a;
    }

    // définir si le jeu est fini
    if ( next_cell == cases )
    {
        SetColor(2);
        cout << player_on->nickname << " gagne, apres " << tours << " rondes!" << endl;

        // convertir le no joueur en string
        ostringstream no;
        no << player_on->no_joueur;
        string number;
        number = no.str();

        string text_message = "Joueur " + number + " gagne!";

        // nom du joueur gagnant
        popup(75, 340, text_message, 95, sf::Color::Yellow);

        restart();
        return;
    }
    // ne rien faire si le no est plus grand que ce qui est possible
    else if ( next_cell > cases )
    {
        cout << player_on->nickname << " got more than he needs! ";

        // message d'erreur
        popup(20, 200, "!>max", 280, sf::Color::Black);

        // clear le message
        display(false);

        // continuer avec le prochain
        next_player();
        return;
    }

    // configurer la position du pion
    tmp_cell = new(struct boite);
    tmp_cell = box_end;

    // trouver la case correspondante
    while ( tmp_cell->number > next_cell )
    {
        tmp_cell = tmp_cell->prev;
    }

    // exécution du code échelle
    if (tmp_cell->event > 0)
    {
        cout << "Ladder to " << (tmp_cell->number + tmp_cell->event) << "! " << endl;
        case_bak = tmp_cell->number + tmp_cell->event;
        booster = tmp_cell->event;
    }

    // exécution du code serpent
    else if (tmp_cell->event < 0)
    {
        cout << "Snake to " << (tmp_cell->number + tmp_cell->event) << "!" << endl;
        case_bak = tmp_cell->number + tmp_cell->event;
        booster = tmp_cell->event;
    }

    // définir les variables locales de position
    int tmpx;
    int tmpy;

    // variables de transition
    int start_x = player_on->posx;
    int start_y = player_on->posy;

    // variables de position transitionnelles
    int mask_x = start_x;
    int mask_y = start_y;

    // intervalle de transition
    int move_x;
    int move_y;

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

    // avant tout, affichons le numéro que le dé a eu
    ostringstream num;
    num << des;
    string res;
    res = num.str();

    // afficher résultat du dé
    popup(280, 100, res, 500, sf::Color::Black);

    // assigner la nouvelle valeur
    player_on->nocase = next_cell;

    // définir la position du pion
    player_on->pion.setPosition(tmpx, tmpy);

    // notifier la nouvelle position
    cout << "Deplace sur case " << player_on->nocase << endl;

    // calcul des intervalles
    move_x = (tmpx - start_x) / 50;
    move_y = (tmpy - start_y) / 50;

    // éviter des bugs si pas de transition nécessaire
    if ( start_x == tmpx ) move_x = 0;
    if ( start_y == tmpy ) move_y = 0;

    // arrêter le son & jouer slide
    son_de.stop();
    son_avance.play();

    // calcul des positions (50 frames de transition)
    for (int frame = 0; frame <= 50; frame ++)
    {
        player_on->posx = mask_x;
        player_on->posy = mask_y;

        // rafraîchir la fenêtre et champ de jeu
        display(false);

        // pour qu'on voie la transition
        Sleep(10);

        // mettre à jour la position suivante
        mask_x = mask_x + move_x;
        mask_y = mask_y + move_y;
    }

    // re-transition si booster found
    if ( case_bak != 0 )
    {
        // jouer sons
        son_avance.stop(); // juste au cas ou
        Sleep(250); // attendre un peu

        // avant tout, affichons le boost
        ostringstream num;
        num << booster;
        string res;
        res = num.str();

        // assigner la nouvelle valeur
        player_on->nocase = case_bak;

        // définir la position du pion
        player_on->pion.setPosition(tmpx, tmpy);

        // recommencer à compter les cases
        tmp_cell = box_end;

        // aller à la bonne case avant qu'on déplace tout
        while ( tmp_cell->number > case_bak )
        {
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

        // console notify & jouer sons
        if (booster > 0)
        {
            popup(280, 100, res, 500, sf::Color::Green);

            // jouer le son
            son_slide.play();
        }
        else if (booster < 0)
        {
            popup(280, 100, res, 500, sf::Color::Red);

            // jouer le son
            son_snake.play();
        }

        // recalcul
        // variables de transition
        start_x = player_on->posx;
        start_y = player_on->posy;

        // variables de position transitionnelles
        mask_x = start_x;
        mask_y = start_y;

        // calcul des intervalles
        move_x = (tmpx - start_x) / 50;
        move_y = (tmpy - start_y) / 50;

        // éviter des bugs si pas de transition nécessaire
        if ( start_x == tmpx ) move_x = 0;
        if ( start_y == tmpy ) move_y = 0;

        // calcul des positions (50 frames de transition)
        for (int frame = 0; frame <= 50; frame ++)
        {
            player_on->posx = mask_x;
            player_on->posy = mask_y;

            // rafraîchir la fenêtre et champ de jeu
            display(false);

            // pour qu'on voie la transition
            Sleep(10);

            // mettre à jour la position suivante
            mask_x = mask_x + move_x;
            mask_y = mask_y + move_y;
        }
    }

    // assigner les nouvelles valeurs de position
    player_on->posx = tmpx;
    player_on->posy = tmpy;

    // changer de joueur
    next_player();

    // afficher le no de joueur actif
    cout << player_on->nickname << "'s turn.";

    // rafraîchir la fenêtre et champ de jeu
    display(false);
}

// cycler à travers les joueurs
void next_player()
{
    if ( player_on->no_joueur == players_global )
    {
        tours ++;
    }

    player_on = player_on->next;
}

void display(bool show_text)
{
    // dessiner le tableau de jeu
    make_table();

    // organiser les serpents et échelles
    make_boosts(); // il y a un problème ici pour la fonction multi-niveaux

    // dessiner le(s) pions sur le tableau (probleme lorsque le joueur quitte le jeu)
    draw_players();

    // si il y a quelquechose à afficher, fais-le!
    if ( show_text != false )
    {
        // dessiner le chiffre
        gamewin.draw(message);
    }

    // afficher les formes
    gamewin.display();

    // effacer tout
    gamewin.clear();

    // affichage du jeu sur la console (petit extra)
    // console_update();
}

/* ------------------------- Section relative à la gestion ----------------------------- */

// affiche les règles du jeu
void rules()
{
    cout << "Les lignes ";
    SetColor(5);
    cout << "mauves ";
    SetColor(7);
    cout << "sont les serpents; les ";
    SetColor(10);
    cout << "vertes ";
    SetColor(7);
    cout << "sont des echelles" << endl;
    cout << endl << "Pour lancer le de, appuyez sur espace" << endl;

    Sleep(1000);
}

int main()
{
    // cacher la fenêtre (très sommaire)
    gamewin.setVisible(false);

    // buffer de son
    sf::SoundBuffer boot;

    // charger fichiers
    if (!boot.loadFromFile("boot.wav")) cout << "Import failed" << endl;

    // définition du son
    sf::Sound boot_sound;

    // éviter des loops
    boot_sound.setLoop(false);

    // assigner le buffer au son
    boot_sound.setBuffer(boot);

    // jouer son au démarrage
    boot_sound.play();

    // Charger une police
    loadFont();

    // afficher les crédits
    SetColor(15);
    cout << "Serpents et Echelles, Programmation III" << endl << "Christian Medel (cmedelahumada@gmail.com) (c) 2016" << endl << endl << endl;
    SetColor(7);

    // compter et créer joueurs
    make_players();

    // niveaux...
    calc_niveau();

    // afficher les regles
    rules();

    // créer et dessnier les éléments SFML
    gamewin.setVisible(true);
    display(false);

    // message de succès sur la console
    cout << "Board traced successfully" << endl;

    // afficher le no de joueur actif
    cout << player_on->nickname << "'s turn.";

    // Évènements pendant que la fenêtre est ouverte (lancer tour, etc)
    while(gamewin.isOpen())
    {
        // lire les évènements fenêtres
        events();
    }
    return 0;
}

void loadFont()
{
    // Charger une police pour l'affichage de numéros
    if (!font.loadFromFile("C:/Windows/Fonts/arial.ttf")) cout << "font not found" << endl;
}

void del_players()
{
    // détruire les pointeurs jusqu'à ce qu'on arrive au début
    while ( joueurs != le_premier )
    {
        delete joueurs;
        joueurs = joueurs->next;
    }
}

void quit()
{
    // fermer la fenêtre
    gamewin.close();

    // musique de fin
    sf::SoundBuffer shut_down;

    // charger fichiers
    if (!shut_down.loadFromFile("quit.wav")) cout << "Import failed" << endl;

    // conteneur du son
    sf::Sound quit;

    // assimiler le son
    quit.setBuffer(shut_down);

    // destruction des structures
    del_players();

    // message de fermeture
    SetColor(15);
    cout << endl << "Au revoir!" << endl;
    quit.play();

    Sleep(4750);
}

void restart()
{
    SetColor(7);

    string ans;
    cout << endl << "Voulez-vous rejouer? (y/n) ";
    cin >> ans;

    if ( ans == "y" )
    {
        // reset the players list
        del_players();

        // reset pour réorganiser les boost
        first_show = true;

        // relaunch
        main();
    }
    else
    {
        // fonction quit
        quit();
    }
}

void quit_player()
{
    // structure de contenance
    TMP = new(struct pions);
    TMP = joueurs;

    // garder le nick pour notifier
    string nick = player_on->nickname;

    // tracking
    int numero = player_on->no_joueur;

    // trouver le joueur à modifier
    int del_joueur = player_on->no_joueur - 1;
    if (del_joueur == 0) del_joueur = players_global;

    cout << numero << " ";

    // naviguer la liste joueurs jusqu'à trouver le bon joueur
    // while (joueurs->no_joueur != player_on->no_joueur)
    while (joueurs->next->no_joueur != numero)
    {
        joueurs=joueurs->next;
    }

    // on a maintenant le joueur qu'on veut détruire
    le_prochain = joueurs->next;

    cout << joueurs << endl;

    // on trouve le joueur avant (erreur!)
    while (joueurs->no_joueur != numero)
    {
        joueurs=joueurs->next;
    }

    cout << joueurs << endl;

    // on remplace le prochain
    joueurs->next = le_prochain;

    // efface le joueur
    delete TMP;

    player_on = joueurs;

    cout << joueurs << "->" << joueurs->next << "->" << joueurs->next->next << endl;
    cout << " " << nick << " left the game. " << player_on->nickname << "'s turn.";
}

// lecture des évènements sur la fenêtre
void events()
{
    sf::Event event;
    while(gamewin.pollEvent(event))
    {
        switch(event.type)
        {
        // La fenêtre se ferme
        case sf::Event::Closed:
            // fonction d'arrêt
            quit();

            break;

        // Une touche est appuyée
        case sf::Event::KeyPressed:

            // identifier la touche appuyée
            switch(event.key.code)
            {

            // La touche 'espace' es appuyée
            case sf::Keyboard::Space:
                // jouer le tour
                make_turn(NULL);
                break;

            // La touche 'espace' es appuyée
            case sf::Keyboard::C:
                // jouer le tour
                make_turn(cases);
                break;

            case sf::Keyboard::Q:
                // quitter la partie
                // quit_player(); ne marche pas

                // recommencer
                restart();
                break;
            }

        // traitement des autres évènemenrs SFML
        default:
            // cout<<"event happenned"<<event.type<<endl;
            break;
        }
    }
}
