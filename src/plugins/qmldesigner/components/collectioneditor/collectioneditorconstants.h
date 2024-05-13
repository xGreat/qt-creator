// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#pragma once

namespace QmlDesigner::CollectionEditorConstants {

enum class SourceFormat { Unknown, Json };

inline constexpr char SOURCEFILE_PROPERTY[]                 = "source";
inline constexpr char ALLMODELS_PROPERTY[]                  = "allModels";
inline constexpr char JSONCHILDMODELNAME_PROPERTY[]         = "modelName";

inline constexpr char COLLECTIONMODEL_IMPORT[]              = "QtQuick.Studio.Utils";
inline constexpr char JSONCOLLECTIONMODEL_TYPENAME[]        = "QtQuick.Studio.Utils.JsonListModel";
inline constexpr char JSONCOLLECTIONCHILDMODEL_TYPENAME[]   = "QtQuick.Studio.Utils.ChildListModel";
inline constexpr char JSONBACKEND_TYPENAME[]                = "JsonData";

inline constexpr QStringView DEFAULT_DATA_JSON_FILENAME     = u"data.json";
inline constexpr QStringView DEFAULT_MODELS_JSON_FILENAME   = u"models.json";
inline constexpr QStringView DEFAULT_DATASTORE_QML_FILENAME = u"DataStore.qml";
inline constexpr QStringView DEFAULT_JSONDATA_QML_FILENAME  = u"JsonData.qml";

} // namespace QmlDesigner::CollectionEditorConstants
