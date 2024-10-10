Project 5A - Concurrent Store
====================

<!-- TODO: Fill this out. -->

## Design Overview:

## Collaborators:

## Extra Credit Attempted:

No extra credit possible.

## How long did it take to complete the Concurrent Store portion of KVStore?

Took about 15 hours to complete this portion of KV store.

<!-- Enter an approximate number of hours that you spent actively working on the project. -->


Project 5B - Distributed Store
====================

<!-- TODO: Fill this out. -->

## Design Overview:

## Collaborators:

## Extra Credit Attempted:

No extra credit given.

## How long did it take to complete the Distributed Store portion of KVStore?

Took about 5 hours to complete this portion of KVStore.

<!-- Enter an approximate number of hours that you spent actively working on the project. -->

1. Who was your stakeholder pair? (<1 sentence)

My stakeholder pair is Angel Yoline and the Film Enthusiasts. 

2. What kind of delete did you implement and why? Explain your decisions as if you were reporting to an ethical auditor who will determine whether your design is justified. Highlight and explain what you think is the most compelling reason that supports the specific kind of deletion you’ve implemented (1-2 short paragraphs)

I implemented a delete function that takes in a user's ID and deletes all the posts related to the user. Users have a right to delete their
data on the platform and therefore was implemented. I am strictly defining someone's personal data as data that they posted. If another user posted a picture or information —— and somehow obtained that —— unfortunately, that no longer qualifies as data from the original user as Tweeter as a platform cannot verify the origin of the data. In addition, this deletion covers both the GDPR Right to be Forgotten Act and the CPPA Right to Delete Act. Users request the deletion of personal data from the platform. Since the user is not being deleted, personal information attached to their profile—such as username—would stay on the platform. Contextualizing this with the given scenario, Angel Yoline requested to delete her own posts. However, she also requested to delete Brad's post, and the public discourse about the controversy. This was not deleted as that was not data she specifically created and would be opposed to freedom of speech laws. The opposing stakeholder, the film enthusiasts, also have a right to information: they want to know what Angel has posted before. In my opinion, the artist cannot be separated from their art. The comments that the artist says—the character of the artist— are prevalent in the art they create. Therefore, the film enthusiasts deserve to know what Angel posted. Since Angel also publically posted this, and to our knowledge, on her own will, this content should be readily available. However, due to the content being information that Angel created, it should be able to be taken down by her. However, posts referencing what she said should not be taken down, as Angel has no rights to those posts.

3. What are the shortcomings of your implementation? Who are some short term and/or long term stakeholders (beyond the ones we’ve asked you to consider) who could be adversely affected by your decision? (1-2 short paragraphs)

One of the shortcomings of this implementation is that it does not delete all personal data. The person's username and any personal information disclosed by other users on the platform will stay. This could expose someone else's personal information, violating private data rights. In addition, this implementation only considered posts as personal data —— there may be other personal data stored on the platform that were not focused on. The short-term stakeholders that the decision may impact include reporters or journalists who want to view the source material for Angel's post. Long-term stakeholders who could be adversely affected by the decision include internet archivists, investigative journalists, and the social media company itself. For one, internet archivists would have less access to data if people decide to delete their data. Therefore, they may be unable to assemble a complete digital archive of a specific topic or controversy. In addition, investigative journalists conducting criminal investigations may also be affected. If people can delete all the data off the platform, this effectively limits their digital footprint, thus leading to a more challenging and fragmented information ecosystem. Finally, users deleting personal information would hurt Tweeter itself as they have less data on their users. The decrease in user information would lead to fewer targeted ads, affecting Tweeter's profitability. 

4. How might your approach to this assignment change if you were asked to consider the interests of other stakeholders not mentioned in the scenario? (1-2 short paragraphs)

If other stakeholder interests were considered, my approach to this assignment would stay the same, with a few potential changes. I believe people have a right to delete the information they post on the Tweeter platform. However, the person has no right to delete other people's posts that may mention them or reveal information about them. Since the platform cannot verify where and how this personal information was given to another person, the posts may not be removed. However, in the case of defamation or an individual delivering false information about someone else, the person being attacked should have the right to delete the posts. Therefore, the implementation should be altered to include this edge case. Additionally, suppose the stakeholders of the company were considered. In that case, each user's data should be anonymized but kept inside the advertising targeting data set to ensure that ad revenue and overall profitability do not decrease. Finally, it is essential to consider how this implementation would affect the general public. On the one hand, it gives people more autonomy over their data and allows them to control their digital presence. On the other hand, it allows bad actors to cover up their tracks and hide their digital presence. Therefore, an alternative implementation could include keeping an encrypted copy of user data in case police or government activity need to use it. All in all, data privacy affects thousands of stakeholders; there needs to be significant analysis and discussion before corporations or governments make data privacy decisions. 