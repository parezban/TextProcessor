/*************************************************************************
 * Copyright © 2012-2015, Targoman.com
 *
 * Published under the terms of TCRL(Targoman Community Research License)
 * You can find a copy of the license file with distributed source or
 * download it from http://targoman.com/License.txt
 *
 *************************************************************************/
/**
 @author S. Mohammad M. Ziabary <smm@ziabary.com>
 @author Behrooz Vedadian <vedadian@aut.ac.ir>
 */

#include <QFile>
#include "IXMLWriter.h"

namespace Targoman {
namespace NLPLibs {
namespace TargomanTP{
namespace Private {

IXMLWriter* IXMLWriter::Instance = NULL;

#define TGMN_SUFFIXES "t|ll|ve|s|m|re|d" //these terms can come after apostrofe

IXMLWriter::IXMLWriter() :
    NormalizerInstance(Normalizer::instance()),
    SpellCorrectorInstance(SpellCorrector::instance())
{
    // Email detection
    this->RxEmail = QRegExp("([A-Za-z0-9._%+-][A-Za-z0-9._%+-]*@[A-Za-z0-9.-][A-Za-z0-9.-]*\\.[A-Za-z]{2,4})");

    this->RxURL = QRegExp(QStringLiteral("(?:(?:https?|ftp)://)?"
                                            "(?:(?!10(?:\\.\\d{1,3}){3})"
                                            //      "(?!127(?:\\.\\d{1,3}){3})"
                                            //      "(?!169\\.254(?:\\.\\d{1,3}){2})"
                                            //      "(?!192\\.168(?:\\.\\d{1,3}){2})"
                                            //      "(?!172\\.(?:1[6-9]|2\\d|3[0-1])(?:\\.\\d{1,3}){2})"
                                            "(?:[1-9]\\d?|1\\d\\d|2[01]\\d|22[0-3])"
                                            "(?:\\.(?:1?\\d{1,2}|2[0-4]\\d|25[0-5])){2}"
                                            "(?:\\.(?:[1-9]\\d?|1\\d\\d|2[0-4]\\d|25[0-4]))"
                                            "|(?:(?:[a-z\u00a1-\uffff0-9]+-?)*[a-z\u00a1-\uffff0-9]+)"
                                            "(?:\\.(?:[a-z\u00a1-\uffff0-9]+-?)*[a-z\u00a1-\uffff0-9]+)*"
                                            "(?:\\.(?:[a-z\u00a1-\uffff]{2,4})))(?::\\d{2,5})?(?:/[^\\s]*)?"),
                          Qt::CaseInsensitive);
    this->RxURLValidator = QRegExp("^" + this->RxURL.pattern() + "$");

    // Abbreviations
    {
        this->RxAbbr = QRegExp("\\b([A-Z]\\.(?:[A-Z\\d]\\.)(?:[A-Z\\d]\\.)*)(?=[^\\w]|$)");
        this->RxAbbrDotless = QRegExp("\\b([A-Z]\\.[A-Z\\d](?:\\.[A-Z\\d])*)\\b");

        this->RxMultiDots = QRegExp(QStringLiteral("(\\.\\.(\\.)*)"));
    }

    // suffixes
    this->RxSuffix = QRegExp(QStringLiteral("((?:'(?:%1))\\b)").arg(TGMN_SUFFIXES));

    //zhnDebug: what is this?
    // Dates
    this->RxDate = QRegExp(QStringLiteral("^$"));
    // Times
    this->RxTime = QRegExp(QStringLiteral("^$"));
    // Numbers
    this->RxDashSeparator = QRegExp("(\\w)\\-(\\w)");
    this->RxUnderlineSeparator = QRegExp("(\\w)\\_(\\w)");
    this->RxSpecialNumber = QRegExp("(\\d+\\.\\d+\\.(?:\\d+\\.?)*|^\\d+\\.)");

    //((?:(?:(?:\\b)(:num:))|(?:(?::num:)(?:\\b)))(?=([^\\.\\d]|\\.(?:[^\\d]|$))))
    this->RxNumberLeft = QRegExp("((?:(?:\\b)(" +
                                 QStringLiteral("[\\+\\-]?[0-9]{1,3}[',](?:[0-9]{3}[',])*[0-9]{3}(?:\\.[0-9][0-9]*)?|"
                                                   "[\\+\\-]?[0-9][0-9]*(?:\\.[0-9][0-9]*)?") +
                                 "))(?=([^\\.\\d]|\\.(?:[^\\d]|$))))");

    this->RxNumberRight = QRegExp("(?:([^a-zA-Z0-9])(" +
                                  QStringLiteral("(?:[\\+\\-])?[0-9]{1,3}[',](?:[0-9]{3}[',])*[0-9]{3}(?:\\.[0-9][0-9]*)?|"
                                                    "(?:[\\+\\-])?[0-9][0-9]*(?:\\.[0-9][0-9]*)?") +
                                  +")(?:\\b))(?=([^\\.\\d]|\\.(?:[^\\d]|$)))");

    this->RxNumberValidator =
            QRegExp("^[\\+\\-]?[0-9]{1,3}[',](?:[0-9]{3}[',])*[0-9]{3}(?:\\.[0-9][0-9]*)?|"
                    "[\\+\\-]?[0-9][0-9]*(?:\\.[0-9][0-9]*)?$");

    this->RxNumbering = QRegExp(QStringLiteral("^((?:\\s)*(?:[\"'\\(\\[`])?(?:\\s)*"
                                                  "(?:(?:\\d+)|"
                                                  "(?:M{0,4}(?:CM|CD|D?C{0,3})(?:XC|XL|L?X{0,3})(?:IX|IV|V?I{0,3}))|"
                                                  "(?:الف|[a-zA-Z]|[ابپتثجچهخدذرزژسشصضطظعغفقکگلمنوهی])"
                                                  ")(?:"
                                                  "(?:[\\-\\.])"
                                                  "(?:(?:\\d+)|"
                                                  "(?:M{0,4}(?:CM|CD|D?C{0,3})(?:XC|XL|L?X{0,3})(?:IX|IV|V?I{0,3}))|"
                                                  "(?:الف|[a-zA-Z]|[ابپتثجچهخدذرزژسشصضطظعغفقکگلمنوهی])"
                                                  "))*"
                                                  "(?:[\\-\\.\\s\\)>])"
                                                  ")"));

    this->RxOrdinalNumber = QRegExp("((?:\\b)(?:1st|2nd|3rd|\\d+th)(?:\\b))");
    this->RxPersianLatin = QRegExp(QStringLiteral("([\u0600-\u06ff])(\\d+)?([a-zA-Z])"));
    this->RxLatinPersian = QRegExp(QStringLiteral("([a-zA-Z])(\\d+)?([\u0600-\u06ff])"));
    this->RxPersianNumber = QRegExp(QStringLiteral("([\u0600-\u06ff])(\\d+)"));
}

/**
 * @brief Reads abbriviation from file and adds them to #RxAbbrDic
 * @param _configFile abbriviation file address.
 */

void IXMLWriter::init(const QString &_configFile)
{
    QString AbbreviationDetectionRegex =
            QStringLiteral("\\b(Mr\\.");

    QFile AbbrF(_configFile);
    AbbrF.open(QIODevice::ReadOnly);
    QByteArray DataLine;
    int CommentIndex = -1;

    while (!AbbrF.atEnd())
    {
        DataLine = AbbrF.readLine().trimmed();
        if (DataLine.isEmpty() || DataLine.startsWith("##"))
            continue;
        if ((CommentIndex = DataLine.indexOf("##")) >= 0)
            DataLine.truncate(CommentIndex);
        AbbreviationDetectionRegex.append("|" + DataLine.replace(".", "\\."));
    }

    //        this->RxAbbrDic = QRegExp (AbbreviationDetectionRegex + ")(?=[^\\w]|$)");
    this->RxAbbrDic = QRegExp (AbbreviationDetectionRegex + ")(?=\\b)");
}

/**
 * @brief finds and tags some patterns in input text and converts them to  ixml format.
 * @param _inStr  input string
 * @param _lang language for input argument of SpellCorrector class.
 * @param _lineNo line number
 * @param _interactive argument of spellCorrector and normalizer. can SpellCorrector or Normalizer class be done interactively or not.
 * @param _useSpellCorrector use spell corrector or not.
 * @return returns converted ixml text.
 */

QString IXMLWriter::convert2IXML(const QString &_inStr,
                                 const QString& _lang,
                                 quint32 _lineNo,
                                 bool _interactive,
                                 bool _useSpellCorrector)
{
    if(_inStr.trimmed().isEmpty())
        return "";
    QString InputPhrase, OutputPhrase;
    InputPhrase += _inStr;

    TargomanDebug(7,"[ORG] "<<InputPhrase);

    //Normalize
    OutputPhrase.clear();
    OutputPhrase.append(" "); // prepend a space before string.

    //normalize input text.
    for (int i=0; i<InputPhrase.size(); i++){
        OutputPhrase.append(
                    this->NormalizerInstance.normalize(
                        InputPhrase.at(i),
                        ((i + 1) < InputPhrase.size() ? InputPhrase.at(i+1) : QChar('\n')),
                        _interactive,
                        _lineNo,
                        InputPhrase,
                        i));
    }
    OutputPhrase+=" ."; //append a space and a dot to the end of string for some bug fixings.


    QStringList LstURL;             //list of founded URLs
    QStringList LstEmail;           //list of founded Emails
    QStringList LstAbbr[3];         //list of founded three kind of abbriviations.
    QStringList LstDate;            //list of founded Dates.
    QStringList LstTime;            //list of founded Times.
    QStringList LstSpecialNumber;   //list of founded Special Numbers.
    QStringList LstOrdinal;         //list of founded Ordinal Numbers.
    QStringList LstNumberLeft;      //list of founded Numbers that were in left side of a word.
    QStringList LstNumberRight;     //list of founded Numbers that were in right side of a word.
    QStringList LstSuffixes;        //list of founded Suffixes.
    QStringList LstOrderedItem;     //list of founded ordered items.
    QStringList LstSymbols;         //list of founded symbols.

    TargomanDebug(7,"[NRM] |"<<OutputPhrase<<"|");
    OutputPhrase.replace("&amp;", " & ").replace("&gt;", " > ").replace("&lt;", " < "); //replace '<' a '>' with some special string in order to prevent errors in xml tags.
    TargomanDebug(7,"[NR2] |"<<OutputPhrase<<"|");
    OutputPhrase.replace(this->RxNumbering, "\\1 ");
    TargomanDebug(7,"[OLI] |"<<OutputPhrase<<"|");
    QStringList PhraseTokens = OutputPhrase.split(" ", QString::SkipEmptyParts);

    // if first token is number we are not sure whether it is for ordered list or not. So we will check it in this if
    if (PhraseTokens.size() && this->RxNumbering.exactMatch(PhraseTokens.first())){
        if (this->RxNumberValidator.exactMatch(PhraseTokens.first())){ //check whether first token is a normal number (numbers with optional thousand seperator or decimal numbers )or not.
            LstNumberLeft.append(PhraseTokens.first());
            PhraseTokens[0] = "TGMNNUL";
        }else if (this->RxURLValidator.exactMatch(PhraseTokens.first())){ //check whether first token is IP of a website or not.
            LstURL.append(PhraseTokens.first());
            PhraseTokens[0] = "TGMNURL";
        }else if (this->RxAbbrDic.exactMatch(PhraseTokens.first())){ //check whether first token is in abbreviation dictionary or not.
            LstAbbr[0].append(PhraseTokens.first());
            PhraseTokens[0] = "TGMNABD";
        }else{  // if first token was non of the above, it is ordered list item.
            LstOrderedItem.append(PhraseTokens.first());
            PhraseTokens[0] = "TGMNOLI";
        }
        OutputPhrase = PhraseTokens.join(" ");
    }
    //adds a space between persian word and number
    OutputPhrase.replace(this->RxPersianNumber, "\\1 \\2");
    TargomanDebug(7,"[P2N] |"<<OutputPhrase<<"|");
    //adds spaces between persian word ,number and latin word.
    OutputPhrase.replace(this->RxPersianLatin, "\\1 \\2 \\3");
    TargomanDebug(7,"[P2L] |"<<OutputPhrase<<"|");
    // it doesn't add space between latin word and number but adds space between number and persian word.
    OutputPhrase.replace(this->RxLatinPersian, "\\1\\2  \\3");
    TargomanDebug(7,"[L2P] |"<<OutputPhrase<<"|");

    //find and replace a list patterns.
    OutputPhrase = this->markByRegex(OutputPhrase, this->RxEmail, "EML", &LstEmail);
    OutputPhrase = this->markByRegex(OutputPhrase, this->RxAbbr, "ABR", &LstAbbr[1]);
    OutputPhrase = this->markByRegex(OutputPhrase, this->RxAbbrDotless, "ABS", &LstAbbr[2]);
    OutputPhrase = this->markByRegex(OutputPhrase, this->RxAbbrDic, "ABD", &LstAbbr[0]);
    OutputPhrase = this->markByRegex(OutputPhrase, this->RxURL, "URL", &LstURL);
    OutputPhrase = this->markByRegex(OutputPhrase, this->RxMultiDots, "MDT", NULL);
    OutputPhrase = this->markByRegex(OutputPhrase, this->RxDate, "DAT", &LstDate);
    OutputPhrase = this->markByRegex(OutputPhrase, this->RxTime, "TIM", &LstTime);
    OutputPhrase = this->markByRegex(OutputPhrase, this->RxOrdinalNumber, "ORD", &LstOrdinal);
    OutputPhrase = this->markByRegex(OutputPhrase, this->RxSpecialNumber, "SNM", &LstSpecialNumber);
    OutputPhrase.replace(this->RxDashSeparator, "\\1 - \\2"); // adds space before and after dashes in string.
    TargomanDebug(7,"[DSH] |"<<OutputPhrase<<"|");
    OutputPhrase.replace(this->RxUnderlineSeparator, "\\1 _ \\2"); // adds space before and after underlines in string.
    TargomanDebug(7,"[UND] |"<<OutputPhrase<<"|");
    OutputPhrase = this->markByRegex(OutputPhrase, this->RxNumberRight,"NUR", &LstNumberRight, 2);
    OutputPhrase = this->markByRegex(OutputPhrase, this->RxNumberLeft, "NUL", &LstNumberLeft);
    OutputPhrase = this->markByRegex(OutputPhrase, this->RxSuffix, "SFX", &LstSuffixes);

    //add space before and after non alphaNumeric characters.
    InputPhrase = OutputPhrase;
    OutputPhrase.clear();
    foreach (const QChar& Char, InputPhrase){
        if (!Char.isLetterOrNumber() && Char != ARABIC_ZWNJ){
            OutputPhrase.append(' ');
            OutputPhrase.append(Char);
            OutputPhrase.append(' ');
        }
        else
            OutputPhrase.append(Char);
    }

    TargomanDebug(7,"[TKN] |"<<OutputPhrase<<"|");

    //for each token, if any letter of a token is a symbol, add that token to list of symbols and replace whole symbol with " TGMNSYM "
    QStringList Tokens = OutputPhrase.split(" ",QString::SkipEmptyParts);
    for (int i=0; i<Tokens.size(); i++) {
        bool IsSymbol = true;
        foreach (const QChar& Ch, Tokens[i])
            if (Ch.category() != QChar::Symbol_Modifier &&
                    Ch.category() != QChar::Symbol_Math &&
                    Ch.category() != QChar::Symbol_Other){
                IsSymbol = false;
                break;
            }
        if (IsSymbol){
            LstSymbols.append(Tokens[i]);
            Tokens[i] == " TGMNSYM ";
        }
    }
    OutputPhrase = Tokens.join(" ");

    TargomanDebug(7,"[SYM] |"<<OutputPhrase<<"|");

    if (_useSpellCorrector)
        OutputPhrase = this->SpellCorrectorInstance.process(_lang, OutputPhrase, _interactive);

    TargomanDebug(7,"[SPL] |"<<OutputPhrase<<"|");


    //replace targoman marks with their corresponding words, wrapped with xml tags.
    InputPhrase = OutputPhrase;
    OutputPhrase.clear();
    foreach (const QString& Token, InputPhrase.split(" ",QString::SkipEmptyParts)) {
        if(Token == "TGMNEML")
            replaceTag(OutputPhrase, enuTextTags::Email, LstEmail.takeFirst());
        else if(Token == "TGMNURL")
            replaceTag(OutputPhrase, enuTextTags::URL, LstURL.takeFirst());
        else if(Token == "TGMNABD")
            replaceTag(OutputPhrase, enuTextTags::Abbreviation, LstAbbr[0].takeFirst());
        else if(Token == "TGMNABR")
            replaceTag(OutputPhrase, enuTextTags::Abbreviation, LstAbbr[1].takeFirst());
        else if(Token == "TGMNABS")
            replaceTag(OutputPhrase, enuTextTags::Abbreviation, LstAbbr[2].takeFirst());
        else if(Token == "TGMNMDT")
            OutputPhrase.append(MULTI_DOT);
        else if(Token == "TGMNDAT")
            replaceTag(OutputPhrase, enuTextTags::Date, LstDate.takeFirst());
        else if(Token == "TGMNTIM")
            replaceTag(OutputPhrase, enuTextTags::Time, LstTime.takeFirst());
        else if(Token == "TGMNORD")
            replaceTag(OutputPhrase, enuTextTags::Ordinals, LstOrdinal.takeFirst());
        else if(Token == "TGMNSNM")
            replaceTag(OutputPhrase, enuTextTags::SpecialNumber, LstSpecialNumber.takeFirst());
        else if(Token == "TGMNNUL")
            replaceTag(OutputPhrase, enuTextTags::Number, LstNumberLeft.takeFirst());
        else if(Token == "TGMNNUR")
            replaceTag(OutputPhrase, enuTextTags::Number, LstNumberRight.takeFirst());
        else if(Token == "TGMNOLI")
            replaceTag(OutputPhrase, enuTextTags::OrderedListItem, LstOrderedItem.takeFirst());
        else if(Token == "TGMNSYM")
            replaceTag(OutputPhrase, enuTextTags::Symbol, LstSymbols.takeFirst());
        else if(Token == "TGMNSFX")
            OutputPhrase.append(LstSuffixes.takeFirst());
        else if(Token == "<")
            OutputPhrase.append("&lt;");
        else if(Token == ">")
            OutputPhrase.append("&gt;");
        else if(Token == "&")
            OutputPhrase.append("&amp;");
        else
            OutputPhrase.append(Token);
        OutputPhrase.append(" ");
    }
    TargomanDebug(7,"[TRP] |"<<OutputPhrase<<"|");

    OutputPhrase.truncate(OutputPhrase.size() - 2);
    OutputPhrase = this->NormalizerInstance.fullTrim(OutputPhrase.replace("  "," ").replace("  "," "));
    TargomanDebug(7,"[ALL-TAGS] |"<<OutputPhrase<<"|");

    return OutputPhrase;
}

/**
 * @brief Finds a RegExp in input _phrase and if found, replaces that with a _mark and adds that to _listOfMaches.
 * @param _phrase input phrase.
 * @param _regex pattern to search for.
 * @param _mark replacement string
 * @param _listOfMatches list to add.
 * @param _capID id of group in regular expression.
 * @return returns replaced string with mark.
 */
QString IXMLWriter::markByRegex(const QString &_phrase,
                                QRegExp &_regex,
                                const QString &_mark,
                                QStringList *_listOfMatches,
                                quint8 _capID)
{
    int Pos=0;
    int Start=0;
    QString OutputPhrase;
    while ((Pos = _regex.indexIn(_phrase, Pos)) != -1) {
        QString A = _regex.cap(_capID);
        if (_listOfMatches)
            _listOfMatches->append(_regex.cap(_capID));
        OutputPhrase += _phrase.mid(Start,Pos - Start) + " " +
                (_capID == 2 ? _regex.cap(1) : "") + " TGMN" + _mark + " ";
        Start = Pos + _regex.matchedLength();
        Pos += _regex.matchedLength();
    }
    OutputPhrase += _phrase.mid(Start);

    TargomanDebug(7,"["<<_mark<<"] |"<<OutputPhrase<<"|");
    return OutputPhrase;
}

}
}
}
}
