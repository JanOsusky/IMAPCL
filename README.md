# IMAPCL - IMAP Klient v C++

**Autor**: Jan Osuský  
**Login**: xosusk00  
**Datum vytvoření**: 25. října 2024  


IMAPCL je aplikace příkazového řádku v C++ pro přístup a stahování emailů z IMAP serveru. Tento program je implementací všech požadavků základního zadání projektu IMAPCL z předmětu ISA. Uživatelé mohou stahovat emailové zprávy a ukládat je do určené složky. Klient je postavený podle protokolu IMAP4rev1, jak je specifikováno v [RFC 3501](https://www.rfc-editor.org/rfc/rfc3501). Program byl otestován na až stažení 1500 emailů.

## Funkce
- Připojení k IMAP serverům a autentizace pomocí uživatelského jména a hesla s možností bezpečného připojení přes SSL.
- Stažení a uložení emailů do specifikované složky.
- Konfigurovatelné přes příkazové argumenty.


## Použití aplikace imapcl

Aplikace `imapcl` slouží k připojení na IMAP server a umožňuje stahování emailů do lokálního adresáře. Syntaxe spuštění aplikace je následující:

`./imapcl server [-p port] [-T [-c certfile] [-C certaddr]] [-n] [-h] -a auth_file [-b MAILBOX] -o out_dir`


## Parametry
- **server**: Povinný parametr. Zadejte název serveru (IP adresa nebo doménové jméno) jako cílový zdroj.
  
- **-p port** *(volitelný)*: Určuje číslo portu na serveru. Doporučuje se vybrat výchozí hodnotu podle typu připojení (např. IMAP nebo IMAPS) a dle registrace u IANA.

- **-T** *(volitelný)*: Aktivuje šifrování (protokol IMAPS). Pokud tento parametr není uveden, použije se nešifrovaná varianta protokolu.

- **-c certfile** *(volitelný)*: Soubor s certifikáty, který se použije pro ověření platnosti SSL/TLS certifikátu, který předkládá server.

- **-C certaddr** *(volitelný)*: Adresář s certifikáty pro ověřování SSL/TLS certifikátu serveru. Pokud není specifikováno, použije se výchozí adresář `/etc/ssl/certs`.

- **-n** *(volitelný)*: Při aktivaci bude aplikace pracovat pouze s novými zprávami.

- **-h** *(volitelný)*: Při aktivaci budou stahovány pouze hlavičky zpráv, nikoli celé zprávy.

- **-a auth_file**: Povinný parametr. Soubor `auth_file` obsahuje informace pro autentizaci (příkaz LOGIN) a slouží k ověření přístupu k serveru.

- **-b MAILBOX** *(volitelný)*: Specifikuje název schránky na serveru, se kterou se bude pracovat. Výchozí hodnota je `INBOX`.

- **-o out_dir**: Povinný parametr. Určuje výstupní adresář, kam se uloží stažené zprávy.

## Příklad použití:

`.\imapcl mail.server.cz -p 993 -T -a auth_file -b INBOXMine -o /home/user/emails`

## Odevzdané soubory
- **README.md**: Tento soubor.
- **main.cpp**: Hlavní zdrojový soubor programu.
- **connect.cpp**, **imap.cpp**: Soubory pro IMAP připojení a zpracování e-mailů.
- **connect.h**, **imap.h**: Hlavičkové soubory pro deklaraci tříd a funkcí.
- **Makefile**: Soubor pro sestavení aplikace.
- **manual.pdf***: Soubor s dokumentací.
