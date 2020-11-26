
#include "StreamFeatures.h"

#include <assert.h>

#include "Network/Xmpp/XmppValues.h"
#include "3rdParty/TinyXml/tinyxml.h"



namespace net {



cXmppStreamFeatures::cXmppStreamFeatures()
: mUnsupportedFeatureCount(0)
{
}// END cXmppStreamFeatures



void cXmppStreamFeatures::Clear()
{
	mFeatures.clear();
	mUnsupportedFeatureCount=0;
}// END Clear



void cXmppStreamFeatures::AddFeature(eStreamFeatureType type, bool required)
{
	sStreamFeature feature;
	feature.type = type;
	feature.required = required;
	mFeatures.push_back(feature);
}// END AddFeature



void cXmppStreamFeatures::PopulateFromStanza(const cStreamFeaturesStanza& stanza)
{
	Clear();

	// SASL mechanisms first
	const TiXmlElement* pMechanismsRoot = stanza.MechanismsNode();
	if(pMechanismsRoot)
	{
		for(const TiXmlNode* child = pMechanismsRoot->FirstChild(); child != NULL; child = child->NextSibling())
		{
			const TiXmlText* textNode = static_cast<const TiXmlText*> (child->FirstChild());
			assert(textNode);

			if(textNode)
			{
				if(textNode->Value() == std::string(ATTRIBUTE_SASL_PLAIN))
				{
					AddFeature(SASL_PLAIN, false);
					continue;		
				}

				if(textNode->Value() == std::string(ATTRIBUTE_SASL_MD5))
				{
					AddFeature(SASL_MD5, false);
					continue;
				}

				printf("Unsupported feature - %s\n", textNode->Value());
			}
		}
	}


	// all other features
	const TiXmlElement* pFeaturesRoot = stanza.RootNode();
	if(!pFeaturesRoot)
	{
		assert(0);
		return;
	}
	for(const TiXmlNode* child = pFeaturesRoot->FirstChild(); child != NULL; child = child->NextSibling())
	{
		// skip sasl
		if(child->Value() == std::string(NODE_MECHANISMS))
		{
			continue;
		}

		if(child->Value() == std::string(NODE_START_TLS))
		{
			AddFeature(TLS, false);
			continue;
		}

		if(child->Value() == std::string(NODE_BIND))
		{
			AddFeature(BIND, false);
			continue;
		}

		if(child->Value() == std::string(NODE_SESSION))
		{
			AddFeature(SESSION, false);
			continue;
		}

		printf("Unsupported feature - %s\n", child->Value());
	}
}// END PopulateFromStanza



const cXmppStreamFeatures::sStreamFeature* cXmppStreamFeatures::Feature(eStreamFeatureType type)
{
	for(u32 i=0; i < mFeatures.size(); i++)
	{
		if(mFeatures[i].type == type)
		{
			return &(mFeatures[i]);
		}
	}
	return NULL;
}// END Feature



bool cXmppStreamFeatures::SupportsFeature(eStreamFeatureType type)
{
	const sStreamFeature* pFeature = Feature(type);
	return (pFeature != NULL);
}// END SupportsFeature



bool cXmppStreamFeatures::RequiresFeature(eStreamFeatureType type)
{
	const sStreamFeature* pFeature = Feature(type);
	return (pFeature && pFeature->required);
}// END RequiresFeature




} // namespace net

