Server page
Page Accueil
    - Bouton Afficher les données instantanées: action -> affiche la page "données instantanées"
    - Bouton Afficher les données mémorisées: action -> affiche la page "données mémorisées"
    - Bouton Mettre en veille -> place l'ESP en mode deep_sleep avec reveil sur horloge

Page Données instantanées
    - Champ texte température froid
    - Champ texte température chaud
    - Champ texte chauffage on/off
    - Bouton home
    Rafraichissement automatique 2s

Page données mémorisées
    - Liste colonnes: date, heure, température froid, température chaud, chauffage
    - Bouton home 


Logiciel ESP 
    - Server WEB
        - Réponse à la requete "/"
            -> envoyer la page Home
        - Réponse à la requete "/immediateData
            -> envoyer la page "Données instantanées"
        - Réponse à la requete "/historizedData
            -> envoyer la page "Données mémorisées"
        - Réponse à la requete "/sleep
            - Mettre le sytème en veille

    - Déclenchement mesure périodique
        - Pendant que le système est en veille
            -> c'est le RTC qui déclenche le réveil et la mesure se fera dans le setup() lors du redémarrage du micro
        - Pendant que le système est actif (serveur WEB affiche des pages et recoit une demande de rafraichissement périodique)
            -> Il doit y avoir un timer périodique créé dans la fonction setup() quand la raison du réveil est de type sérial

    - Gestion mise en veille
        - Arret des timers (timeout veille, et mesure périodique)
        - appel de la function de mise en deep_sleep avec un intervalle calculé entre la date courante et la date de la prochaine mesure

    - Gestion du reveil
        - dans la fonction setup()
            - Lire le weakup_reason qui peut être de type serial, timer ou défaut? (cas du boot normal)
            - Si timer:
                - Faire la mesure et stocker le résultat brut (date, heure, température froid, température chaud, etat chauffage)
                - Mettre le systeme en veille
            - Si serial:
                - Démarrer le server WEB et attendre un certain temps une connexion
                - Démarrer un timer qui a pour but de déclencher les mesures périodiques
                - Si pas de connexion dans un certain délai, mettre le système en veille

    - Gestion persistence mesures
        - Prévoir un stockage en RAM d'une taille raisonnable (ex 60 mesures = 1h, compromis taille mémoire / nombre d'accèes)
        - Utiliser un pointeur à avance automatique pour accès la zone à écrire
        - Ecrire les données dans un fichier en utilisant FFS quand le stockage est rempli et reinitialiser le pointeur en début de stockage.
        - L'écriture doit se faire pendant la phase active
        - L'écriture en fichier doit être faite quand l'affichage des données mémorisées est demandé.
        - Dans le fichier on stocke les données en format brute.

    - Lecture températures

    - Lecture etat chauffe

interesting commands
sudo /Library/Application\ Support/Wireshark/ChmodBPF/ChmodBPF

curl -d " -H "Content-Type: application/x-www-form-urlencoded"   -X GET http://192.168.4.22/getInstantValues

-d: is same as --data-ascii (using this flag, the default content-type sent to the server is application/x-www-form-urlencoded)
-i: show the HTTP response headers
-v: verbose (response and protocol details)
-G or --get are same

curl -G -i -d 'timeEpoch=1921' http://192.168.4.22/setTime

curl -G -i http://192.168.4.22/getInstantValues

curl -G -i -d 'count=10&offset=10'  http://192.168.4.22/history


a investiguer
[466348][D][AsyncTCP.cpp:187] _get_async_event(): coalescing polls, network congestion or async callbacks might be too slow!
Dans la fenetre du monitor quand le client sollicite valeurs instantanées