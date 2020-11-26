
#ifndef XMPP_STREAM_FEATURES_H
#define XMPP_STREAM_FEATURES_H


#include <vector>

#include "Network/Xmpp/Stanza.h"

namespace net {



class cXmppStreamFeatures
{
public:
	
	cXmppStreamFeatures();
	virtual ~cXmppStreamFeatures() {}

private:
	cXmppStreamFeatures (const cXmppStreamFeatures& rhs);
	const cXmppStreamFeatures& operator= (const cXmppStreamFeatures& rhs);
	bool operator== (const cXmppStreamFeatures& rhs) const;


public:

	typedef enum
	{
		TLS =0,
		SASL_PLAIN,
		SASL_MD5,
		BIND,
		SESSION
	}eStreamFeatureType;	

	typedef struct  
	{
		eStreamFeatureType type;
		bool required;
	}sStreamFeature;

	void Clear();
	void AddFeature(eStreamFeatureType type, bool required);
	
	void PopulateFromStanza(const cStreamFeaturesStanza& stanza);

	const sStreamFeature* Feature(eStreamFeatureType type);
	
	bool SupportsFeature(eStreamFeatureType type);
	bool RequiresFeature(eStreamFeatureType type);

private:

	std::vector<sStreamFeature> mFeatures;

	u32 mUnsupportedFeatureCount;
};




} // namespace net


#endif // XMPP_STREAM_FEATURES_H