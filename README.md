# PDS_Project
- Tutti i pacchetti vanno mandati con TCP

## entrambi
- 1 classe Database per eseguire le funzioni sui DB

## Client
- [x] Monitoraggio ricorsivo cartella in background
- [ ] Cartella di partenza
- [ ] Salvare in una struttura dati nome e hash di ogni file (usare old_map e new_map aggiornate ogni secondo o ogni modifica)
- [ ] Struttura ad albero per le directory
- [ ] Programma deve runnare in background
- [ ] Quando aggiungo un file o una directory modificare le strutture dati
- [ ] Quando old != new c'è stata una modifica nei file (delete o rename o modifica del contenuto o new) 
- [ ] Quando old != new per modifica, bisogna rendere il file invalido e sincronizzarlo con il server per poi rimetterlo a valido 
- [ ] Fare la push delle modifiche sul server
- [ ] Trasferimento file (Compresso ?) 
- [ ] Trasferimento cartelle (es. mandare il messaggio "DIR /path/to/directory")
- [ ] Gestione errori (se pc o server non online mantenere l'elenco delle modifiche da mandare)
- [ ] Sincronizzazione all'avvio e creazione DB come su server

## Server
- [ ] In ascolto su PORT
- [ ] In base al messaggio bisogna fare un'azione
- [ ] Controllo errori e in caso affermativo risincronizzazione
- [ ] Usare DB
- [ ] Trasferimento file (Compresso ?)
- [ ] Gestione multiclient

DATABASE

DIR(_id_,path)
FILE(_id_,nome,_idDir_,hash)

## Comments
All'avvio, il client chiede di sincronizzarsi dopo essersi autenticato: il server manda il server_file.db( contentente DIR e FILE del database del client che fa la richiesta ) e il relativo hash. Se l'hash di client_file.db mantenuto dal client è uguale a server_file.db non fare niente, altrimenti controllare nel client_file.db i record diversi e mandare le relative richieste.
Una volta terminata la sincronizzazione, il client controlla la cartella vittima ogni 't' secondi, se ci sono modifiche rilevate viene mandata la richiesta al server per mandare la modifica al server. 

SERVER                     CLIENT

    
    <- chiedere sync (invia id_client)
    -> server_file.db - hash di file.db (se hash su client è uguale a hash mandato da server tutto ok, altrimenti...)
    <- request per aggiornare versione client

## Link
SQLITE3: https://www.tutorialspoint.com/sqlite/sqlite_c_cpp.htm
SHA: https://www.cryptopp.com/wiki/SHA

## Comandi
sudo apt-get install libcrypto++-dev libcrypto++-doc libcrypto++-utils
 
## Client - Server protocol
```
Client                    Server
--- Syncronization ---
SYN <username> (Socket::syncRequest) ->
<- <hash di username.db> (Socket::rcvSyncRequest)
************ ALTERNATIVA A SOPRA ******************
SYN <username> (Socket::syncRequest) ->
<- SYN-OK / SYN-ERROR (se username errato)
opt
    SYN-OK ->
    <- <hash di username.db> (Socket::rcvSyncRequest)
***************************************************
alt (if hashServer == hashClient)
    DONE ->
    ------
    <cercare file e directory che non sono giuste>
    loop
        opt (file o cartella non aggiornati)
            <capire se è file o directory e che tipo di operazione bisogna fare>
            CHANGE [FILE/DIR] <file/directory path> <operation> ->
            //con directory non sono necessari altri passaggi
            opt (se è un file ed è stato creato/modificato)
                <- READY <file path> //
                <mandare il file (Socket::sendFile)> ->
            <- DONE <file/directory path>
----------------------
--- Normal usage -----
<modifica rilevata>
CHANGE [FILE/DIR] <file/directory path> <operation> ->
//con directory non sono necessari altri passaggi
opt (se è un file ed è stato creato/modificato)
    <- READY <file path> //
    <mandare il file (Socket::sendFile)> ->
<- DONE <file/directory path>
----------------------
               
```

***ESEMPIO DI GESTIONE DIRECTORIES E FILES***

```

std::shared_ptr<Directory> my_root = Directory::getRoot();
my_root->addFile("file.txt", "AAA");
std::shared_ptr<Directory> dir = my_root->addDirectory("prova5");
std::shared_ptr<File> file = dir->addFile("file3.txt", "BBB");
dir->addDirectory("prova7");
my_root->addDirectory("prova6");
if (dir->removeDir("prova7"))
     std::cout<<"Cartella cancellata correttamente"<<std::endl;
else
     std::cout<<"Problema nel cancellare la cartella"<<std::endl;

if (my_root->renameDir("prova5", "provaRename"))
    std::cout<<"Cartella rinominata correttamente"<<std::endl;
else
    std::cout<<"Problema nel rinominare la cartella"<<std::endl;
    
if (dir->renameFile("file3.txt", "fileRename.txt"))
    std::cout<<"File rinominato correttamente"<<std::endl;
else
    std::cout<<"Problema nel rinominare il file"<<std::endl;

my_root->ls(4);

----------------------
               
```

- Directory::getRoot() -> ritorna il puntatore alla directory di nome ROOT (costante definita dentro directory.cpp)
- dir->addDirectory("name")/dir->addFile("name.txt", "Ha$h") -> aggiungono rispettivamente una directory o un file della cartella dir e ne ritornano il puntatore. Entrambe richiamano i relativi costruttori di directory (makeDirectory (...)) e di file (File(...)).
- dir->removeDir("name")/dir->removeFile("name.txt") -> rimuovono rispettivamente una directory o un file dalla cartella dir e ritornano true se l'operazione è andata con successo, false altrimenti
- dir->removeDir("name")/dir->removeFile("name.txt") -> rinominano rispettivamente una directory o un file dalla cartella dir e ritornano true se l'operazione è andata con successo, false altrimenti
- dir->ls(indent) -> stampa ricorsiva (con intendazione indent) a partire dalla cartella dir
    
