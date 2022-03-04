# První projekt z předmětu IPK 2021/2022

Autor: Hung Do (xdohun00)
## Dokumentace projektu

Úkolem je naprogramovat jednoduchý server v jazyce C/C++, který za použití protokolu 
HTTP komunikuje s klientem. Server naslouchá na zadaném portu a
posílá zpátky různé informace o systému podle požadavků v URL.

### Překlad programu pomoci Makefile
Uživatel může použít přiložený `Makefile` pro překlad programu.
```bash
# prikaz pro preklad programu
make
# prikaz pro predkal program a spusteni na portu 12345 (mozno zmeni v Makefile)
make run
```

Je možno zapnout logování zpráv pomocí dopsání `-DDEBUG` do Makefile.
```
...
CFLAGS=-std=c99 -Wall -Wextra -g -pedantic -DDEBUG
...
```

### Překlad programu ručně
Pro ruční překlad (do spustitelného program `hinfosvc`) je možno použít tyto příkazy:
```bash
# vlozeni -DDEBUG nakonec pro logovani
gcc -o hinfosvc main.c responses.c
```

### Spuštění programu
Program `hinfosvc` přijímá parametr `[port]`. Jde o kladnou celočíselnou
hodnotu v rozmezi 0-65535 značící port pro naslouchání (doporučeno použít hodnoty vetší než 1023).  
Přiklad spuštění programu:
```bash
# server nasloucha na portu 12345
./hinfosvc 12345
```

### Požadavky a příklady
Server reaguje na 3 požadavky:
- hostname - vrací jmého hostujicího serveru
- cpu-name - vrací jmého procesoru na daném serveru
- load - vrací aktuální zátěž serveru  
Na ostatní požadavky vrací chybu 400 (Bad Request).
 
Lokální server  
Terminál 1
```
./hinfosvc 8080
```

Terminál 2
```bash
curl http://localhost:8080/hostname
thinkpad-e14
curl http://localhost:8080/cpu-name
AMD Ryzen 7 4700U with Radeon Graphics
curl http://localhost:8080/load
10.125%
```

Webový prohlížeč  
`http://localhost:8080/hostname`
