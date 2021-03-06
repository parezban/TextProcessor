/******************************************************************************
 * Targoman: A robust Machine Translation framework               *
 *                                                                            *
 * Copyright 2014-2018 by ITRC <http://itrc.ac.ir>                            *
 *                                                                            *
 * This file is part of Targoman.                                             *
 *                                                                            *
 * Targoman is free software: you can redistribute it and/or modify           *
 * it under the terms of the GNU Lesser General Public License as published   *
 * by the Free Software Foundation, either version 3 of the License, or       *
 * (at your option) any later version.                                        *
 *                                                                            *
 * Targoman is distributed in the hope that it will be useful,                *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of             *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the              *
 * GNU Lesser General Public License for more details.                        *
 * You should have received a copy of the GNU Lesser General Public License   *
 * along with Targoman. If not, see <http://www.gnu.org/licenses/>.           *
 *                                                                            *
 ******************************************************************************/
/**
 * @author S. Mohammad M. Ziabary <ziabary@targoman.com>
 * @author Behrooz Vedadian <vedadian@targoman.com>
 * @author Saeed Torabzadeh <saeed.torabzadeh@targoman.com>
 */

#ifndef TARGOMAN_NLPLIBS_TARGOMANTP_PRIVATE_NORMALIZER_H
#define TARGOMAN_NLPLIBS_TARGOMANTP_PRIVATE_NORMALIZER_H

#include <QHash>
#include <QSet>
#include <QStringList>
#include <QVariant>

#include "../TextProcessor.h"

namespace Targoman {
namespace NLPLibs {
namespace TargomanTP{
namespace Private {

TARGOMAN_ADD_EXCEPTION_HANDLER(exNormalizer, exTextProcessor);

TARGOMAN_DEFINE_ENHANCED_ENUM(enuDicType,
                              WhiteList,
                              ReplacingCharacters,
                              RemovingCharacters,
                              SpaceCharacters,
                              ZeroWidthSpaceCharacters,
                              NotSure,
                              EndOfFile
                              );

#define ARABIC_ZWNJ                QChar(0x200C)
#define ARABIC_YE                  QChar(0x06CC)
#define ARABIC_YE_HAMZA            QChar(0x0626)
#define ARABIC_ALEF_HAMZA_DOWN     QChar(0x0625)
#define ARABIC_ALEF_HAMZA_UP       QChar(0x0623)
#define ARABIC_ALEF                QChar(0x0627)

#define MULTI_DOT       QStringLiteral("…")
#define SYMBOL_REMOVED  QChar(0x2205)

class Normalizer
{
public:
    void init(const QString& _configFile, bool _binaryMode = false);
    /**
     * @brief Makes an instance of Normalizer class if it is not instantiated. If it is instantiated, returns that instance.
     */
    static Normalizer& instance(){
        static Normalizer*      Instance = NULL;
        return *(Q_LIKELY(Instance) ? Instance : (Instance = new Normalizer));
    }

    QString normalize(const QChar& _char,
                      const QChar& _nextChar,
                      bool _interactive,
                      quint32 _line,
                      const QString& _phrase,
                      size_t _charPos,
                      bool _skipRecheck = false);

    QString normalize(const QString& _string, qint32 _line = -1, bool _interactive = false);

    void updateBinTable(const QString& _binFilePath, bool _interactive = false);

    static QString fullTrim(const QString& _str);
    /**
     * @brief removes spaces and zero width non joiners from both sides of input string.
     * @param input string
     */
    static QString sidesTrim(const QString& _str){
        QString Trimmed = _str.trimmed();
        if (Trimmed.isEmpty())
            return "";
        if(_str.at(Trimmed.size() - 1) == ARABIC_ZWNJ)
            Trimmed.truncate(Trimmed.size() - 1);
        if(_str.at(0) == ARABIC_ZWNJ)
            Trimmed = Trimmed.remove(0,1);
        return Trimmed.trimmed();
    }


private:
    Normalizer();
    Q_DISABLE_COPY(Normalizer)

    void add2Configs(enuDicType::Type _type, QChar _originalChar, QChar _replacement = QChar());
    QString char2Str(const QChar &_char, bool _hexForced = false);
    QList<QChar> str2QChar(QString _str, int _line, bool _allowRange = true);

private:
    QHash<QChar,QString>    ReplacingTable;             /** < A Map to contain chars that should be replaced. Content of this variable will be added using Normalization config file. */
    QList<QVariant>         BinTable;                   /** < A List that has normalize form of all character. Index of normal form of each character in this list, is its unicode value.*/
    QSet<QChar>             WhiteList;                  /** < A Set to contain valid chars. Content of this variable will be added using Normalization config file. */
    QSet<QChar>             RemovingList;               /** < A Set to contain invalid chars. Content of this variable will be added using Normalization config file. */
    QSet<QChar>             SpaceCharList;              /** < A Set to contain all kind of spaces chars. Content of this variable will be added using Normalization config file. */
    QSet<QChar>             ZeroWidthSpaceCharList;     /** < A Set to contain all kind of zero width spaces chars. Content of this variable will be added using Normalization config file. */
    QString                 ConfigFileName;                 /** < Configuration file address */
    bool                    BinaryMode;                 /** < If Normalization data is in binary mode this variable will be true.*/
    QChar                   LastChar;                   /** < Last character in normalization process.*/
};

}
}
}
}

#endif // TARGOMAN_NLPLIBS_TARGOMANTP_PRIVATE_NORMALIZER_H
