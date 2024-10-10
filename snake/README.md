CSCI 0300 Project 1 - Snake
===========================

## Design Overview:

None

## Collaborators:

None

## Responsible Computing:
Partner CS login: plestz

Question 1:

The consortium was founded by an engineer from Xerox (Joe Becker) and two engineers from Apple (Lee Collins and Mark Davis).

The current members of the consortium include software and technology companies, government ministries, research institutions, 
user groups, and individuals interested in Unicode. The technology company members include Apple, Microsoft, Amazon, and Meta, among many
others. The government ministries are from Bangladesh, Oman, and India. Research institutions include UC Berkeley.

This group of members presents a lot of Western bias in the updates of Unicode. Most of the companies are headquartered in the US, with
a few other represented countries such as China, Oman, Bangladesh, and India. 5 out of 195 countries are being represented, and therefore, there
may be decisions that have explicit political or cultural benefits. In addition, the main members are large American corporations which are primarily motivated by financial gain. This may result in decisions that favor these companies and further their profit-motivated goals.

Question 2:

A language Unicode still needs to support is called Loma, which the Loma people speak in Liberia and Guinea. The entire language is missing from Unicode and presents many issues for the people speaking the language. For one, creating a website in Loma becomes challenging as it's not natively available in Unicode. Therefore, a developer must use pictures or an alternative form of presenting a language. Therefore, internet accessibility for these people is almost impossible as typical applications such as Google, Google Maps, or Microsoft Word would be impossible to use. This affects these people's ability to develop and prosper as a nation.

Question 3:
a.
     The position for Han Unification is that according to Unicode, "graphologically the script has been long considered a single script." While the writing systems of Japan, Korea, and China are much different, they all share a "single Han script" as part of their writing system. The Japanese, Korean, and Chinese writing systems are complex; however, the Japanese Kanji writing system, for example, uses the same script as the Chinese Hanzi system. The memory and maintenance costs of the equivalence tables are very high, and therefore, the Unicode developers decided not to implement these differences. 

b.
    The overall conclusion that we came to is while the Han Unification is efficient for memory and maintenance costs, that does not outweigh
    the cultural and language barriers that it creates. Initially, I presented my points on the benefits of Han Unification. Mainly, I discussed how Unicode prioritized efficiency and sustainability in its decision. The decision was efficient as fewer Unicode characters were being used, making the entire Unicode package smaller. In addition, the decision was sustainable because Unicode has a smaller memory footprint, which allows cloud storage space to be allocated for other uses, decreasing the need for more data centers. Fewer data centers lead to a smaller environmental footprint. Both my partner and I agree that Unicode prioritized efficiency and sustainability above all. However, my partner presented convincing arguments about the challenges that Japanese, Chinese, and Korean writers face when working with Han Unification. These three
    languages have other individual glyphs that appear in their unique scripts. Native speakers are also normalized to seeing their language-specific glyphs, so it would be challenging to read their language with a script they're not used to seeing. This presents additional
    barriers to people who are less educated, as they would have a more challenging time reading unfamiliar website text. This would create a large technology gap between educated and uneducated people, leading to more social inequality. 

## Extra Credit attempted:

The extra credit attempted was the following: grow more than one length on each food, making the snake solid green, and making the food
look like food. To enable extra credit, you have to enter 1 as a second input when running the file.  

I created a #define variable called snake_grows and set it to 2. When extra credit is enabled, the snake grows by length two each time.

To make the snake solid green, I looked at the ncurses documentation and simulated a similar look as the wall, except green.

Similarly for food, I looked up the unicode value for a kiwi emoji and used that for the food visuals.

Hope you like it :)

## How long did it take to complete Snake?

30 hours

<!-- Enter an approximate number of hours that you spent actively working on the project. -->
