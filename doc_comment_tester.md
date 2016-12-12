# Comment tester les modifications de filemap.c avec l'image qemu

Cette documentation indique comment procéder rapidement pour tester les modifications de filemap.c.

## Compiler test_mmap.c

L'arborescence actuelle de l'image ressemble à ceci:

* /home/armadeus
  * Logiciels/armadeus-5.2 : dossier où se trouve le compilateur
  * qemu : dossier depuis lequel on lance qemu
    * linux-2.6.29 : dossier d'où on compile le noyau

Pour compiler test_mmap, il faut faire:
```shell
cd Logiciels/armadeus-5.2
source armadeus_env.sh
arm-linux-gcc test_filemap.c -o test_filemap
```

## Lancer qemu

Maintenant on fait Ctrl+Shift+T pour ouvrir un nouveau terminal.

Faire:
```shell
cd ~/qemu
./launch_qemu
```

Puis à nouveau Ctrl+Shift+T et:
```shell
ssh root@10.0.2.XX
``` où XX correspond à l'adresse de l'image qemu

## Copier dans la vm

Depuis le premier onglet, on copie l'exécutable avec: 
```shell
scp test_filemap root@10.0.2.XX:/root
```

On copie aussi le fichier de données que le fichier de test lit avec:
```shell
scp ~/qemu/linux-2.6.29/MAINTAINERS root@10.0.2.XX:/root
```

## Tester
On exécute depuis le 3e onglet:
```shell
./test_filemap
```

Sur la fenêtre de la machine qemu devraient s'afficher les différents messages noyau.

__Le cache étant rempli une première fois, il faut le vider avec la commande:__

```shell
sync && echo 3 > /proc/sys/vm/drop_caches
```

___Attention: la fenêtre de la machine qemu ne s'actualise pas toujours, il faut appuyer sur Entrée pour forcer l'apparition des messages___