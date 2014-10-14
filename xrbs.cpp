#include <stdlib.h>
#include <iterator>

#include "rapidxml.hpp"
#include "rapidxml_utils.hpp"
#include "rapidxml_print.hpp"
#include "rapidxml_iterators.hpp"

using namespace rapidxml;

void rapidxml::parse_error_handler(const char *what, void *where)
{
}

xml_document<char> doc;

char * documentBuffer = NULL;

class StationData
{
public:
	StationData(xml_node<> * node)
	{
		m_node = node;
		m_id[0] = 0;
		m_owner[0] = 0;
	}

	char		m_id[32];
	char		m_owner[32];
	xml_node<> *	m_node;
};

std::vector<StationData *> stations;
std::vector<xml_node<> *> knowns;

xml_node<> * SubscriptionsNode = NULL;
xml_node<> * MemoryNode = NULL;
xml_node<> * ScanNode = NULL;

void FindStations(xml_node<> * root)
{
	xml_node<> * node = root->first_node();

	while(node)
	{
		if (strcmp("memory", node->name()) == 0)
		{
			MemoryNode = node;
		}
		else if (strcmp("scan", node->name()) == 0)
		{
			if (MemoryNode)
				ScanNode = node;
		}
		else if (strcmp("subscriptions", node->name()) == 0)
		{
			SubscriptionsNode = node;
		}
		else if (strcmp("component", node->name()) == 0)
		{
			xml_attribute<> * a = node->first_attribute();

			while (a)
			{
				if (strcmp(a->name(), "class") == 0)
				{
					if (strcmp(a->value(), "station") == 0)
					{
						knowns.push_back(node);
						stations.push_back(new StationData(node));
					}
					else if (strcmp(a->value(), "cluster") == 0)
						knowns.push_back(node);
					else if (strcmp(a->value(), "sector") == 0)
						knowns.push_back(node);
					else if (strcmp(a->value(), "zone") == 0)
						knowns.push_back(node);
					else if (strcmp(a->value(), "gate") == 0)
						knowns.push_back(node);
					else if (strcmp(a->value(), "highway") == 0)
						knowns.push_back(node);
					else if (strcmp(a->value(), "highwayexitgate") == 0)
						knowns.push_back(node);
					else if (strcmp(a->value(), "highwayentrygate") == 0)
						knowns.push_back(node);
				}

				a = a->next_attribute();
			}
		}

		FindStations(node);

		node = node->next_sibling();
	}
}

void KnowIt(xml_node<> * node)
{
	xml_attribute<> * a;

	a = node->first_attribute();

	while (a)
	{
		if (strcmp(a->name(), "knownto") == 0)
			return;

		a = a->next_attribute();
	}

	a = node->document()->allocate_attribute("knownto", "player");
	node->append_attribute(a);
}

void FindStationDetails(StationData * station)
{
	xml_node<> * n;
	xml_attribute<> * a;

	a = station->m_node->first_attribute();

	while (a)
	{
		if (strcmp(a->name(), "id") == 0)
		{
			strcpy(station->m_id, a->value());
		}
		else if (strcmp(a->name(), "owner") == 0)
		{
			strcpy(station->m_owner, a->value());
		}

		a = a->next_attribute();
	}

	if (station->m_id[0] == 0)
		return;

	if (strcmp(station->m_owner, "xenon") == 0)
	{
	}
	else if (strcmp(station->m_owner, "familyryak") == 0)
	{
	}
	else if (strcmp(station->m_owner, "reivers") == 0)
	{
	}
	else
	{
		if (SubscriptionsNode)
		{
			n = SubscriptionsNode->document()->allocate_node(node_element, "object");
			SubscriptionsNode->append_node(n);
			a = SubscriptionsNode->document()->allocate_attribute("id", station->m_id);
			n->append_attribute(a);
		}
	}

	if (ScanNode == NULL)
		return;

	n = ScanNode->document()->allocate_node(node_element, "item");
	ScanNode->append_node(n);
	a = ScanNode->document()->allocate_attribute("component", station->m_id);
	n->append_attribute(a);
	a = ScanNode->document()->allocate_attribute("level", "0");
	n->append_attribute(a);

	n = station->m_node->first_node();

	while (n)
	{
		if (strcmp("connections", n->name()) == 0)
		{
			n = n->first_node();

			while (n)
			{
				a = n->first_attribute();

				while (a)
				{
					if (strcmp(a->name(), "macro") == 0)
					{
						if (a->value()[14] == 's')
						{
							xml_node<> * nn;
							xml_attribute<> * aa;

							nn = ScanNode->document()->allocate_node(node_element, "item");
							ScanNode->append_node(nn);
							aa = ScanNode->document()->allocate_attribute("component", station->m_id);
							nn->append_attribute(aa);
							aa = ScanNode->document()->allocate_attribute("connection", a->value());
							nn->append_attribute(aa);
							aa = ScanNode->document()->allocate_attribute("level", "0");
							nn->append_attribute(aa);
						}

						break;
					}

					a = a->next_attribute();
				}

				n = n->next_sibling();
			}

			break;
		}

		n = n->next_sibling();
	}
}

void documentLoad(const char * dname)
{
	FILE * f = fopen(dname, "r");

	if (f)
	{
		fseek(f, 0, SEEK_END);
		int bytes = ftell(f);
		fseek(f, 0, SEEK_SET);

		documentBuffer = (char *)malloc(bytes + 1);

		if (documentBuffer)
		{
			fread(documentBuffer, 1, bytes, f);

			documentBuffer[bytes] = 0;

			doc.parse<parse_full>(documentBuffer);

			xml_node<> * root = doc.first_node("savegame");

			if (root)
			{
				FindStations(root);

				if (MemoryNode)
				{
					if (SubscriptionsNode)
					{
						if (MemoryNode->parent() != SubscriptionsNode->parent())
						{
							printf("MemoryNode->parent() != SubscriptionsNode->parent()\n");
							exit(1);
						}

						SubscriptionsNode->parent()->remove_node(SubscriptionsNode);
					}

					SubscriptionsNode = MemoryNode->document()->allocate_node(node_element, "subscriptions");
					MemoryNode->parent()->insert_node(MemoryNode, SubscriptionsNode);

					if (ScanNode)
					{
						if (ScanNode->parent() != MemoryNode)
						{
							printf("ScanNode->parent() != MemoryNode\n");
							exit(1);
						}

						MemoryNode->remove_node(ScanNode);
					}

					ScanNode = MemoryNode->document()->allocate_node(node_element, "scan");
					MemoryNode->append_node(ScanNode);
				}

				for (unsigned int i = 0; i < stations.size(); i++)
				{
					FindStationDetails(stations[i]);
				}

				for (unsigned int i = 0; i < knowns.size(); i++)
				{
					KnowIt(knowns[i]);
				}
			}

			std::string so;
			print(std::back_inserter(so), doc, rapidxml::print_no_indenting);

			char dname2[256];

			sprintf(dname2, "%s_", dname);

			FILE * f = fopen(dname2, "wb+");

			if (f)
			{
				fwrite(so.c_str(), 1, so.size(), f);
				fclose(f);
			}
		}

		fclose(f);
	}
}

int main(int argc, char* argv[])
{
	if (argc == 2)
	{
		documentLoad(argv[1]);
	}
	else
	{
		documentLoad("quicksave.xml");
	}
}
