/***************************************************************************
            qgsogrprovider.h Data provider for ESRI shapefile format
                    Formerly known as qgsshapefileprovider.h
begin                : Oct 29, 2003
copyright            : (C) 2003 by Gary E.Sherman
email                : sherman at mrcc.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsrectangle.h"
#include "qgsvectordataprovider.h"
#include "qgsvectorfilewriter.h"
#include "qgsvectorlayerimport.h"

class QgsField;
class QgsVectorLayerImport;

class QgsOgrFeatureIterator;

#include <ogr_api.h>

#if defined(GDAL_VERSION_NUM) && GDAL_VERSION_NUM >= 1800
#define TO8(x)   (x).toUtf8().constData()
#define TO8F(x)  (x).toUtf8().constData()
#define FROM8(x) QString::fromUtf8(x)
#else
#define TO8(x)   (x).toLocal8Bit().constData()
#define TO8F(x)  QFile::encodeName( x ).constData()
#define FROM8(x) QString::fromLocal8Bit(x)
#endif

/**
  \class QgsOgrProvider
  \brief Data provider for ESRI shapefiles
  */
class QgsOgrProvider : public QgsVectorDataProvider
{
    Q_OBJECT

  public:

    /** convert a vector layer to a vector file */
    static QgsVectorLayerImport::ImportError createEmptyLayer(
      const QString& uri,
      const QgsFields &fields,
      QGis::WkbType wkbType,
      const QgsCoordinateReferenceSystem *srs,
      bool overwrite,
      QMap<int, int> *oldToNewAttrIdxMap,
      QString *errorMessage = 0,
      const QMap<QString, QVariant> *options = 0
    );

    /**
     * Constructor of the vector provider
     * @param uri  uniform resource locator (URI) for a dataset
     */
    QgsOgrProvider( QString const & uri = "" );

    /**
     * Destructor
     */
    virtual ~QgsOgrProvider();

    virtual QgsCoordinateReferenceSystem crs();

    /**
     * Sub-layers handled by this provider, in order from bottom to top
     *
     * Sub-layers are used when the provider's source can combine layers
     * it knows about in some way before it hands them off to the provider.
     */
    virtual QStringList subLayers() const;

    /**
     *   Returns the permanent storage type for this layer as a friendly name.
     */
    virtual QString storageType() const;

    virtual QgsFeatureIterator getFeatures( const QgsFeatureRequest& request );

    /** Accessor for sql where clause used to limit dataset */
    virtual QString subsetString();

    virtual bool supportsSubsetString() { return true; }

    /** mutator for sql where clause used to limit dataset size */
    virtual bool setSubsetString( QString theSQL, bool updateFeatureCount = true );

    /**
     * Get feature type.
     * @return int representing the feature type
     */
    virtual QGis::WkbType geometryType() const;

    /** return the number of layers for the current data source

    @note

    Should this be subLayerCount() instead?
    */
    virtual size_t layerCount() const;

    /**
     * Get the number of features in the layer
     */
    virtual long featureCount() const;

    /**
     * Get the field information for the layer
     */
    virtual const QgsFields & fields() const;

    /** Return the extent for this data layer
     */
    virtual QgsRectangle extent();

    /** Update the extents
     */
    virtual void updateExtents();

    /**Writes a list of features to the file*/
    virtual bool addFeatures( QgsFeatureList & flist );

    /**Deletes a feature*/
    virtual bool deleteFeatures( const QgsFeatureIds & id );

    /**
     * Adds new attributes
     * @param attributes list of new attributes
     * @return true in case of success and false in case of failure
     * @note added in 1.2
     */
    virtual bool addAttributes( const QList<QgsField> &attributes );

    /**
     * Deletes existing attributes
     * @param attributes a set containing names of attributes
     * @return true in case of success and false in case of failure
     */
    virtual bool deleteAttributes( const QgsAttributeIds &attributes );

    /**Changes attribute values of existing features */
    virtual bool changeAttributeValues( const QgsChangedAttributesMap & attr_map );

    /**Changes existing geometries*/
    virtual bool changeGeometryValues( QgsGeometryMap & geometry_map );

    /**Tries to create a .qix index file for faster access if only a subset of the features is required
     @return true in case of success*/
    virtual bool createSpatialIndex();

    /**Create an attribute index on the datasource*/
    virtual bool createAttributeIndex( int field );

    /** Returns a bitmask containing the supported capabilities
        Note, some capabilities may change depending on whether
        a spatial filter is active on this provider, so it may
        be prudent to check this value per intended operation.
        See the OGRLayer::TestCapability API for details.
      */
    virtual int capabilities() const;

    virtual void setEncoding( const QString& e );


    /** return vector file filter string

      Returns a string suitable for a QFileDialog of vector file formats
      supported by the data provider.  Naturally this will be an empty string
      for those data providers that do not deal with plain files, such as
      databases and servers.

      @note

      It'd be nice to eventually be raster/vector neutral.
    */
    /* virtual */
    QString fileVectorFilters() const;
    /** return a string containing the available database drivers */
    QString databaseDrivers() const;
    /** return a string containing the available directory drivers */
    QString protocolDrivers() const;
    /** return a string containing the available protocol drivers */
    QString directoryDrivers() const;

    /**Returns true if this is a valid shapefile
    */
    bool isValid();

    /** Returns the minimum value of an attribute
     *  @param index the index of the attribute */
    QVariant minimumValue( int index );

    /** Returns the maximum value of an attribute
     *  @param index the index of the attribute */
    QVariant maximumValue( int index );

    /** Return the unique values of an attribute
     *  @param index the index of the attribute
     *  @param values reference to the list of unique values */
    virtual void uniqueValues( int index, QList<QVariant> &uniqueValues, int limit = -1 );

    /** return a provider name

    Essentially just returns the provider key.  Should be used to build file
    dialogs so that providers can be shown with their supported types. Thus
    if more than one provider supports a given format, the user is able to
    select a specific provider to open that file.

    @note

    Instead of being pure virtual, might be better to generalize this
    behavior and presume that none of the sub-classes are going to do
    anything strange with regards to their name or description?

    */
    QString name() const;


    /** return description

      Return a terse string describing what the provider is.

      @note

      Instead of being pure virtual, might be better to generalize this
      behavior and presume that none of the sub-classes are going to do
      anything strange with regards to their name or description?

     */
    QString description() const;

    /** Returns true if the provider is strict about the type of inserted features
          (e.g. no multipolygon in a polygon layer)
          @note: added in version 1.4*/
    virtual bool doesStrictFeatureTypeCheck() const { return false;}

    /** return OGR geometry type */
    static int getOgrGeomType( OGRLayerH ogrLayer );

  protected:
    /** loads fields from input file to member attributeFields */
    void loadFields();

    /** find out the number of features of the whole layer */
    void recalculateFeatureCount();

    /** tell OGR, which fields to fetch in nextFeature/featureAtId (ie. which not to ignore) */
    void setRelevantFields( bool fetchGeometry, const QgsAttributeList& fetchAttributes );

    /** convert a QgsField to work with OGR */
    static bool convertField( QgsField &field, const QTextCodec &encoding );

  private:
    unsigned char *getGeometryPointer( OGRFeatureH fet );
    QString ogrWkbGeometryTypeName( OGRwkbGeometryType type ) const;
    OGRwkbGeometryType ogrWkbGeometryTypeFromName( QString typeName ) const;
    QgsFields mAttributeFields;
    OGRDataSourceH ogrDataSource;
    void *extent_;

    /**This member variable receives the same value as extent_
     in the method QgsOgrProvider::extent(). The purpose is to prevent a memory leak*/
    QgsRectangle mExtentRect;
    OGRLayerH ogrLayer;
    OGRLayerH ogrOrigLayer;

    //! path to filename
    QString mFilePath;

    //! layer name
    QString mLayerName;

    //! layer index
    int mLayerIndex;

    /** Optional geometry type for layers with multiple geometries,
     *  otherwise wkbUnknown */
    OGRwkbGeometryType mOgrGeometryTypeFilter;

    //! current spatial filter
    QgsRectangle mFetchRect;

    //! String used to define a subset of the layer
    QString mSubsetString;

    // OGR Driver that was actually used to open the layer
    OGRSFDriverH ogrDriver;

    // Friendly name of the OGR Driver that was actually used to open the layer
    QString ogrDriverName;

    bool valid;
    //! Flag to indicate that spatial intersect should be used in selecting features
    bool mUseIntersect;
    int geomType;
    long featuresCounted;

    mutable QStringList mSubLayerList;

    /** Flag whether OGR will return fields required by nextFeature() calls.
        The relevant fields are first set in select(), however the setting may be
        interferred by some other calls. This flag ensures they are set again
        to correct values.
     */
    bool mRelevantFieldsForNextFeature;

    /**Adds one feature*/
    bool addFeature( QgsFeature& f );
    /**Deletes one feature*/
    bool deleteFeature( QgsFeatureId id );

    QString quotedIdentifier( QString field );

    /**Calls OGR_L_SyncToDisk and recreates the spatial index if present*/
    bool syncToDisc();

    friend class QgsOgrFeatureIterator;
    QgsOgrFeatureIterator* mActiveIterator; //!< pointer to currently active iterator (0 if none)
};
