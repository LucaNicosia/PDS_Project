# PDS_Project
- Tutti i pacchetti vanno mandati con TCP

## entrambi
- 1 classe Database per eseguire le funzioni sui DB

## Client
<span style="color:blue"> - 1 Monitoraggio ricorsivo cartella in background </span>
- 2 Cartella di partenza
- 3 Salvare in una struttura dati nome e hash di ogni file (usare old_map e new_map aggiornate ogni secondo o ogni modifica)
- 4 Struttura ad albero per le directory
- 5 Programma deve runnare in background
- 6 Quando aggiungo un file o una directory modificare le strutture dati
<span style="color:red">- 7 Quando old != new c'è stata una modifica nei file (delete o rename o modifica del contenuto o new) </span>
<span style="color:red">- 8 Quando old != new per modifica, bisogna rendere il file invalido e sincronizzarlo con il server per poi rimetterlo a valido </span>
- 9 Fare la push delle modifiche sul server
<span style="color:blue">- 10 Trasferimento file (Compresso ?) </span>
- 11 Trasferimento cartelle (es. mandare il messaggio "DIR /path/to/directory")
- 12 Gestione errori (se pc o server non online mantenere l'elenco delle modifiche da mandare)
- 13 Sincronizzazione all'avvio e creazione DB come su server

## Server
<span style="color:blue">- 1 In ascolto su PORT</span>
- 2 In base al messaggio bisogna fare un'azione
- 3 Controllo errori e in caso affermativo risincronizzazione
<span style="color:blue">- 4 Usare DB</span>
- 5 Trasferimento file (Compresso ?)
- 6 Gestione multiclient

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
Client                    Server
SYN <username> (Socket::syncRequest) ->
<- <hash di username.db> (Socket::rcvSyncRequest)
<span style="color:green">
SYN <username> (Socket::syncRequest) ->
<- SYN-OK / SYN-ERROR (se username errato)
opt
    SYN-OK ->
    <- <hash di username.db> (Socket::rcvSyncRequest)
</span>
alt (if hashServer == hashClient)
    DONE ->
    ------
    <cercare file e directory che non sono giuste> <span style="color:green">aggiungere tempo modifica a file e directory su DB</span>
    loop
        opt (file o cartella non aggiornati)
            alt (file)
            