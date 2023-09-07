

#include "helper_funcs.h"
using namespace std;

vector<vector<int>> find_intersection(vector<vector<int>> slots1, vector<vector<int>> slots2)
{
   vector<vector<int>> result;
   // iterate through two slots
   int i = 0, j = 0;

   while (i < slots1.size() && j < slots2.size())
   {
      // start and end of each single slot
      int start = max(slots1[i][0], slots2[j][0]);
      int end = min(slots1[i][1], slots2[j][1]);

      if (start < end)
      {
         result.push_back({start, end});
      }

      if (slots1[i][1] <= slots2[j][1])
      {
         i++;
      }
      else
      {
         j++;
      }
   }

   return result;
}

string slotVector_to_str(vector<vector<int>> slot)
{
   if (slot.size() == 0)
   {
      return "none";
   }
   stringstream ss;
   int i;
   for (i = 0; i < slot.size() - 1; i++)
   {
      ss << '[' << slot[i][0] << ", " << slot[i][1] << "], ";
   }
   ss << '[' << slot[i][0] << ", " << slot[i][1] << "]";

   return ss.str();
}

vector<vector<int>> slotStr_to_vector(string slotStr)
{

   vector<vector<int>> slots;
   vector<int> singleSlot;

   int i = 0;
   while (i < slotStr.length() - 2)
   {
      int time;
      if (isdigit(slotStr[i]))
      {
         if (isdigit(slotStr[i + 1]))
         {
            time = stoi(slotStr.substr(i, 2));
            i++;
         }
         else
         {
            time = static_cast<int>(slotStr[i]) - static_cast<int>('0');
         }
         singleSlot.push_back(time);
      }
      i++;

      if (singleSlot.size() == 2)
      {
         slots.push_back(singleSlot);
         singleSlot.clear();
      }
   }
   return slots;
}

// add a comma and space between the names
string namesVector_to_str(vector<string> names)
{
   stringstream ss;
   int i = 0;
   while (i < names.size() - 1)
   {
      ss << names[i] << ", ";
      i++;
   }
   ss << names[i];

   return ss.str();
}

// the nameStr is comma and space separated
// vector<string> nameStr_to_vector(stringstream ss)
// {
//    vector<string> nameVector;
//    string name;

//    while (getline(ss, name, ','))
//    {
//       name.erase(remove_if(name.begin(), name.end(), ::isspace), name.end());
//       nameVector.push_back(name);
//    }
//    return nameVector;
// }