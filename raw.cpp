        // créer la nouvelle cellule et assigner taille
        cell = new(struct boite);
		cell_ant = new(struct boite);

        // pour vérification ultérieure
        if ( cnt == cases )
        {
            box_end = new(struct boite);
            box_end = cell;
        }

        // enregistrer la case précédente
        cell->prev = cell_ant;

        // actualiser la case précédente
        cell_ant = cell;