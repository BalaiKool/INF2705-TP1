# TP4 - Applications Créatives

Le projet de ce TP est d'implémenter un crystal dans un scène de notre choix. Nous avons décider de partir 

## Texture

Afin d'appliquer la texture, nous avons importé le modèle sur Blender, puis nous avons créer un matériel procédural adapté au crystal, avant d'en exporter ses trois composantes: 
- La couleur, uni dans la modélisation blender
- La "roughness" qui permet de donner plus de texture en régissant avec la lumière
- La normale qui permet de simuler du volume sur un objet pourtant plat.

La carte de "roughness" est une carte grisée, donc nous n'aviosn besoin d'utiliser qu'une seule de ses composante lors de son utilsisation dans les shaders.

Tel qu'expliqué dans le cours, la carte normale est une association des canaux RGB à un déplacement simulé du fragment en XYZ. Cette transformation est gérée aussi dans les shaders.

## Génération de terrain & nuages

Le terrain en relief et les nuages sont générés sans textures associés de manière procédurale. 

Le terrain est généré de manière procédural. Il utilise les shaders de Tesselation (TCS et TES) aifn de subdiviser de manière optimale le sol. Puis en utilisant du bruit, nous arrivons à faire varier la hauteur du sol de manière aléatoire et cohérente. Le sol effectue de l'auto-ombrage en fonction des reliefs, avec un effet de brouillard pour atténuer le sol lointain.

Les nuages sont générés de manière procédurale à partir d’un icosaèdre subdivisé dont les sommets sont perturbés pour obtenir une forme organique. Chaque nuage est animé indépendamment avec un déplacement, une rotation lente et une oscillation verticale simulant un effet de flottement. Un système de durée de vie gère leur apparition et leur dissipation via un fade-in/fade-out progressif, évitant tout saut visuel. Le rendu utilise une transparence simple et légère, permettant d’afficher de nombreux nuages animés simultanément avec un faible coût GPU.

## Illumination

Pour éclairer la scène, nous avons choisi d'utiliser un soleil, qui éclaire les nuages et la sol en dessous, tout en projetant des ombres sur le sol. 

De plus, des éclairs sont simulés dans le fond en faisant varier la couleur du fond parmis des valuers de gris. Les éclairs apparaissent de manière aléatoire lorsque la musique est coupée, et en rythme sur la musique lors qu'elle est en trian de jouer.

## Audio

Nousa vons choisi d'utilsier un fond audio qui affectait la scène en gérant l'apparition des éclairs en fonction du volume de la musique. Ainsi, pricipalement sur les temps forts et les "drops" de la musique, des éclairs apparaissent. Nous avons choisi une musique de [Lofi Girl](https://www.youtube.com/@LofiGirl) car elle était rythmé (facilitant la visibilité de la syncrhonisation des éclairs) sans pour autant s'imposer sur la scène.

## Contrôles

Pour se déplacer dans la scène, deux modes de clavier sont possibles, AZERTY et QWERTY. Par défaut, l eclavier QWERTY est utilisé, et l'on peu alterner rapidement entre les claviers grâce à la touche T. Dans chacun des claviers, voici les controles: 
| Action                         | Clavier QWERTY | Clavier AZERTY |
|--------------------------------|----------------|----------------|
| Quitter l'application           | ESC            | ESC            |
| Changer de clavier              | T              | T              |
| Déplacer la caméra vers l'avant | W              | Z              |
| Déplacer la caméra vers l'arrière | S            | S              |
| Déplacer la caméra vers la gauche | A            | Q              |
| Déplacer la caméra vers la droite | D            | D              |
| Déplacer la caméra vers le bas  | Q/ CTRL_gauche | A / CTRL_gauche|
| Déplacer la caméra vers le haut | E/ MAJ_gauche   | E / MAJ_gauche|
| Activer/désactiver la souris   | Espace         | Espace         |
| Tourner la caméra               | Flèches        | Flèches        |
| Tourner la caméra               | Souris         | Souris         |


La partie suivante est relative à l'utilisaiton de l'interface ImGui. Cette interface se décompose en plusieurs parties.

### Contrôle du son

Nous avons choisi d'utiliser une musique de [Lofi Girl](https://www.youtube.com/@LofiGirl) afin d'accompagner notre projet. Nous pouvons démarrer la musique, auquel cas, les éclairs se syncronisent automatiquement à la musique; ou la mettre en pause ou l'arreter complètement. 

### Paramêtres du cristal

Il y a trois facteurs que nous pouvosn crontroler du cristal : 
- Sa vitesse de rotation
- Sa fréquence d'oscillation de haut en bas (en Hz)
- L'amplitude de l'oscillation de haut en bas

### Paramètre d'illumination

Un source de lumière immitant el soleil est présente et est paramétrable. Nous pouvons notamment chosisi la couleur de la source d'illumination, donnant plusieurs ambiances à la scène, l'intensité de la source lumineuse, mais aussi la direction qu'elle pointe. 

On peut choisir de totalement désactiver cette lumière, ainsi que les omrbes projetées sur le sol.


## Imports effectués

- Pour l'audio, nous avons importé SFML/audio.hpp, afin de gérer le traitement du son.
