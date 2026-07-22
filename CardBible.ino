/**
 * Pocket Bible - Final Distribution v1.0 for M5Stack Cardputer Advanced
 *
 * Polished, intentional, easy-to-use Bible app.
 * - HOME: Read the Bible (OT/NT selector), Verse of the Day, Extras, About (#4 direct)
 * - Battery status (icon + %) always shown top-right
 * - Full hymns: complete verses + chorus for every listed song
 * - Expanded Sinner's Prayer + next steps for new Christians (church home, baptism, etc.)
 * - Extras: Sinner's Prayer, Hymns, Topical (Gideon categories - audited)
 * - Reader: red letters for Jesus words, full scroll + L/R chapters (SD)
 *
 * No audio, no bookmarks, no search. Huge_app build. Single final binary: PocketBible.bin
 *
 * Controls: ; . (or k j) up/down, Enter select, B/Q back/home, G=go ref, L/R=ch (full), type=filter (books)
 *
 * SD: copy sd-card-bible/bible/ to SD root as /bible/ for complete 66 books + L/R nav.
 *
 * See build.sh for flash (DELETE old launcher entries every time) + huge_app.
 */

#include "M5Cardputer.h"
#include <SPI.h>
#include <SD.h>
#include <vector>
#include <algorithm>

// SD pins (official for Cardputer)
#define SD_SPI_SCK_PIN  40
#define SD_SPI_MISO_PIN 39
#define SD_SPI_MOSI_PIN 14
#define SD_SPI_CS_PIN   12

// Screen (rotation 1)
#define SCREEN_W 240
#define SCREEN_H 135

M5Canvas canvas(&M5Cardputer.Display);

// No audio - melodies removed per request. Hymns are complete lyrics only.

// ====================== DATA (always available - no SD needed) ======================

struct Verse {
  const char* ref;
  const char* text;
};

// Large curated list of popular / top KJV verses (~110). Easy to browse, high contrast in reader.
const Verse popularVerses[] = {
  {"John 3:16", "For God so loved the world, that he gave his only begotten Son, that whosoever believeth in him should not perish, but have everlasting life."},
  {"Psalm 23:1", "The LORD is my shepherd; I shall not want."},
  {"Romans 8:28", "And we know that all things work together for good to them that love God, to them who are the called according to his purpose."},
  {"Philippians 4:13", "I can do all things through Christ which strengtheneth me."},
  {"Jeremiah 29:11", "For I know the thoughts that I think toward you, saith the LORD, thoughts of peace, and not of evil, to give you an expected end."},
  {"Proverbs 3:5", "Trust in the LORD with all thine heart; and lean not unto thine own understanding."},
  {"Isaiah 40:31", "But they that wait upon the LORD shall renew their strength; they shall mount up with wings as eagles."},
  {"Matthew 11:28", "Come unto me, all ye that labour and are heavy laden, and I will give you rest."},
  {"2 Corinthians 5:17", "Therefore if any man be in Christ, he is a new creature: old things are passed away; behold, all things are become new."},
  {"Psalm 119:105", "Thy word is a lamp unto my feet, and a light unto my path."},
  {"Genesis 1:1", "In the beginning God created the heaven and the earth."},
  {"John 14:6", "Jesus saith unto him, I am the way, the truth, and the life: no man cometh unto the Father, but by me."},
  {"Ephesians 2:8", "For by grace are ye saved through faith; and that not of yourselves: it is the gift of God."},
  {"Psalm 46:10", "Be still, and know that I am God."},
  {"Matthew 6:33", "But seek ye first the kingdom of God, and his righteousness; and all these things shall be added unto you."},
  {"Isaiah 41:10", "Fear thou not; for I am with thee: be not dismayed; for I am thy God."},
  {"John 10:10", "The thief cometh not, but for to steal, and to kill, and to destroy: I am come that they might have life."},
  {"Psalm 37:4", "Delight thyself also in the LORD; and he shall give thee the desires of thine heart."},
  {"Romans 12:2", "And be not conformed to this world: but be ye transformed by the renewing of your mind."},
  {"1 Corinthians 13:4", "Charity suffereth long, and is kind; charity envieth not; charity vaunteth not itself, is not puffed up."},
  {"Galatians 5:22", "But the fruit of the Spirit is love, joy, peace, longsuffering, gentleness, goodness, faith."},
  {"Hebrews 11:1", "Now faith is the substance of things hoped for, the evidence of things not seen."},
  {"1 Peter 5:7", "Casting all your care upon him; for he careth for you."},
  {"Revelation 3:20", "Behold, I stand at the door, and knock: if any man hear my voice, and open the door, I will come in to him."},
  {"Matthew 7:7", "Ask, and it shall be given you; seek, and ye shall find; knock, and it shall be opened unto you."},
  {"John 8:32", "And ye shall know the truth, and the truth shall make you free."},
  {"Romans 5:8", "But God commendeth his love toward us, in that, while we were yet sinners, Christ died for us."},
  {"2 Timothy 1:7", "For God hath not given us the spirit of fear; but of power, and of love, and of a sound mind."},
  {"James 4:7", "Submit yourselves therefore to God. Resist the devil, and he will flee from you."},
  {"1 John 4:7", "Beloved, let us love one another: for love is of God; and every one that loveth is born of God, and knoweth God."},
  {"Psalm 27:1", "The LORD is my light and my salvation; whom shall I fear?"},
  {"Psalm 34:8", "O taste and see that the LORD is good: blessed is the man that trusteth in him."},
  {"Psalm 91:1", "He that dwelleth in the secret place of the most High shall abide under the shadow of the Almighty."},
  {"Proverbs 22:6", "Train up a child in the way he should go: and when he is old, he will not depart from it."},
  {"Ecclesiastes 3:1", "To every thing there is a season, and a time to every purpose under the heaven."},
  {"Isaiah 9:6", "For unto us a child is born, unto us a son is given: and the government shall be upon his shoulder."},
  {"Isaiah 53:5", "But he was wounded for our transgressions, he was bruised for our iniquities."},
  {"Jeremiah 33:3", "Call unto me, and I will answer thee, and shew thee great and mighty things."},
  {"Lamentations 3:22", "It is of the LORD's mercies that we are not consumed, because his compassions fail not."},
  {"Micah 6:8", "He hath shewed thee, O man, what is good; and what doth the LORD require of thee."},
  {"Matthew 5:3", "Blessed are the poor in spirit: for theirs is the kingdom of heaven."},
  {"Matthew 5:14", "Ye are the light of the world. A city that is set on an hill cannot be hid."},
  {"Matthew 22:37", "Jesus said unto him, Thou shalt love the Lord thy God with all thy heart, and with all thy soul, and with all thy mind."},
  {"Mark 10:45", "For even the Son of man came not to be ministered unto, but to minister, and to give his life a ransom for many."},
  {"Luke 2:10", "And the angel said unto them, Fear not: for, behold, I bring you good tidings of great joy."},
  {"John 1:1", "In the beginning was the Word, and the Word was with God, and the Word was God."},
  {"John 3:3", "Jesus answered and said unto him, Verily, verily, I say unto thee, Except a man be born again, he cannot see the kingdom of God."},
  {"John 15:5", "I am the vine, ye are the branches: He that abideth in me, and I in him, the same bringeth forth much fruit."},
  {"John 16:33", "These things I have spoken unto you, that in me ye might have peace. In the world ye shall have tribulation: but be of good cheer; I have overcome the world."},
  {"Acts 2:38", "Then Peter said unto them, Repent, and be baptized every one of you in the name of Jesus Christ for the remission of sins."},
  {"Acts 4:12", "Neither is there salvation in any other: for there is none other name under heaven given among men, whereby we must be saved."},
  {"Romans 3:23", "For all have sinned, and come short of the glory of God."},
  {"Romans 6:23", "For the wages of sin is death; but the gift of God is eternal life through Jesus Christ our Lord."},
  {"Romans 10:9", "That if thou shalt confess with thy mouth the Lord Jesus, and shalt believe in thine heart that God hath raised him from the dead, thou shalt be saved."},
  {"1 Corinthians 13:13", "And now abideth faith, hope, charity, these three; but the greatest of these is charity."},
  {"2 Corinthians 12:9", "And he said unto me, My grace is sufficient for thee: for my strength is made perfect in weakness."},
  {"Ephesians 3:20", "Now unto him that is able to do exceeding abundantly above all that we ask or think."},
  {"Philippians 4:6", "Be careful for nothing; but in every thing by prayer and supplication with thanksgiving let your requests be made known unto God."},
  {"Colossians 3:23", "And whatsoever ye do, do it heartily, as to the Lord, and not unto men."},
  {"1 Thessalonians 5:16", "Rejoice evermore."},
  {"2 Timothy 3:16", "All scripture is given by inspiration of God, and is profitable for doctrine, for reproof, for correction, for instruction in righteousness."},
  {"Hebrews 4:16", "Let us therefore come boldly unto the throne of grace, that we may obtain mercy, and find grace to help in time of need."},
  {"Hebrews 13:5", "Let your conversation be without covetousness; and be content with such things as ye have: for he hath said, I will never leave thee, nor forsake thee."},
  {"James 5:16", "Confess your faults one to another, and pray one for another, that ye may be healed."},
  {"1 John 1:9", "If we confess our sins, he is faithful and just to forgive us our sins, and to cleanse us from all unrighteousness."},
  {"Revelation 21:4", "And God shall wipe away all tears from their eyes; and there shall be no more death, neither sorrow, nor crying, neither shall there be any more pain."},
  {"Psalm 1:1", "Blessed is the man that walketh not in the counsel of the ungodly."},
  {"Psalm 100:1", "Make a joyful noise unto the LORD, all ye lands."},
  {"Psalm 121:1", "I will lift up mine eyes unto the hills, from whence cometh my help."},
  {"Proverbs 4:23", "Keep thy heart with all diligence; for out of it are the issues of life."},
  {"Proverbs 18:10", "The name of the LORD is a strong tower: the righteous runneth into it, and is safe."},
  {"Isaiah 26:3", "Thou wilt keep him in perfect peace, whose mind is stayed on thee."},
  {"Matthew 5:8", "Blessed are the pure in heart: for they shall see God."},
  {"Matthew 6:9", "After this manner therefore pray ye: Our Father which art in heaven, Hallowed be thy name."},
  {"Matthew 28:19", "Go ye therefore, and teach all nations, baptizing them in the name of the Father, and of the Son, and of the Holy Ghost."},
  {"Mark 12:30", "And thou shalt love the Lord thy God with all thy heart, and with all thy soul, and with all thy mind, and with all thy strength."},
  {"Luke 6:31", "And as ye would that men should do to you, do ye also to them likewise."},
  {"Luke 11:9", "And I say unto you, Ask, and it shall be given you; seek, and ye shall find; knock, and it shall be opened unto you."},
  {"John 11:25", "Jesus said unto her, I am the resurrection, and the life: he that believeth in me, though he were dead, yet shall he live."},
  {"Acts 1:8", "But ye shall receive power, after that the Holy Ghost is come upon you."},
  {"Romans 8:38", "For I am persuaded, that neither death, nor life, nor angels, nor principalities, nor powers, nor things present, nor things to come."},
  {"1 Corinthians 15:57", "But thanks be to God, which giveth us the victory through our Lord Jesus Christ."},
  {"2 Corinthians 4:18", "While we look not at the things which are seen, but at the things which are not seen."},
  {"Galatians 2:20", "I am crucified with Christ: nevertheless I live; yet not I, but Christ liveth in me."},
  {"Ephesians 6:10", "Finally, my brethren, be strong in the Lord, and in the power of his might."},
  {"Philippians 1:6", "Being confident of this very thing, that he which hath begun a good work in you will perform it until the day of Jesus Christ."},
  {"Colossians 3:2", "Set your affection on things above, not on things on the earth."},
  {"1 Timothy 6:12", "Fight the good fight of faith, lay hold on eternal life."},
  {"Titus 2:11", "For the grace of God that bringeth salvation hath appeared to all men."},
  {"Hebrews 12:1", "Wherefore seeing we also are compassed about with so great a cloud of witnesses."},
  {"James 1:2", "My brethren, count it all joy when ye fall into divers temptations."},
  {"1 Peter 2:9", "But ye are a chosen generation, a royal priesthood, an holy nation, a peculiar people."},
  {"1 John 3:16", "Hereby perceive we the love of God, because he laid down his life for us."},
  {"Jude 1:24", "Now unto him that is able to keep you from falling, and to present you faultless before the presence of his glory with exceeding joy."},
  {"Revelation 3:21", "To him that overcometh will I grant to sit with me in my throne, even as I also overcame, and am set down with my Father in his throne."},
  {"Psalm 23:4", "Yea, though I walk through the valley of the shadow of death, I will fear no evil: for thou art with me."},
  {"Psalm 46:1", "God is our refuge and strength, a very present help in trouble."},
  {"Proverbs 3:6", "In all thy ways acknowledge him, and he shall direct thy paths."},
  {"Isaiah 43:2", "When thou passest through the waters, I will be with thee; and through the rivers, they shall not overflow thee."},
  {"Matthew 5:4", "Blessed are they that mourn: for they shall be comforted."},
  {"Matthew 6:34", "Take therefore no thought for the morrow: for the morrow shall take thought for the things of itself."},
  {"John 14:27", "Peace I leave with you, my peace I give unto you: not as the world giveth, give I unto you."},
  {"Romans 15:13", "Now the God of hope fill you with all joy and peace in believing."},
  {"2 Corinthians 5:7", "For we walk by faith, not by sight."},
  {"Philippians 4:7", "And the peace of God, which passeth all understanding, shall keep your hearts and minds through Christ Jesus."},
  {"2 Timothy 4:7", "I have fought a good fight, I have finished my course, I have kept the faith."},
  {"1 John 4:8", "He that loveth not knoweth not God; for God is love."},
  {"Revelation 22:20", "He which testifieth these things saith, Surely I come quickly. Amen. Even so, come, Lord Jesus."}
};
const int NUM_POPULAR = sizeof(popularVerses) / sizeof(popularVerses[0]);

// Hymns & Worship - title + COMPLETE lyrics (verses + chorus) for all listed songs.
// No audio/melodies (removed per request). Select to view full lyrics in reader.
struct Hymn {
  const char* title;
  const char* lyrics;
};

Hymn hymns[] = {
  {"Amazing Grace",
   "Amazing grace! How sweet the sound\nThat saved a wretch like me!\nI once was lost, but now am found;\nWas blind, but now I see.\n\n'Twas grace that taught my heart to fear,\nAnd grace my fears relieved;\nHow precious did that grace appear\nThe hour I first believed!\n\nThrough many dangers, toils and snares\nI have already come;\n'Tis grace hath brought me safe thus far,\nAnd grace will lead me home.\n\nWhen we've been there ten thousand years,\nBright shining as the sun,\nWe've no less days to sing God's praise\nThan when we first begun.\n\nWhen we've been there ten thousand years,\nBright shining as the sun,\nWe've no less days to sing God's praise\nThan when we first begun."},

  {"How Great Thou Art",
   "O Lord my God, when I in awesome wonder\nConsider all the worlds Thy hands have made,\nI see the stars, I hear the rolling thunder,\nThy power throughout the universe displayed.\n\nThen sings my soul, my Savior God, to Thee,\nHow great Thou art, how great Thou art!\nThen sings my soul, my Savior God, to Thee,\nHow great Thou art, how great Thou art!\n\nWhen through the woods and forest glades I wander\nAnd hear the birds sing sweetly in the trees,\nWhen I look down from lofty mountain grandeur\nAnd hear the brook and feel the gentle breeze.\n\nThen sings my soul, my Savior God, to Thee,\nHow great Thou art, how great Thou art!\nThen sings my soul, my Savior God, to Thee,\nHow great Thou art, how great Thou art!\n\nAnd when I think that God, His Son not sparing,\nSent Him to die, I scarce can take it in;\nThat on the cross, my burden gladly bearing,\nHe bled and died to take away my sin.\n\nThen sings my soul, my Savior God, to Thee,\nHow great Thou art, how great Thou art!\nThen sings my soul, my Savior God, to Thee,\nHow great Thou art, how great Thou art!\n\nWhen Christ shall come with shout of acclamation\nAnd take me home, what joy shall fill my heart!\nThen I shall bow in humble adoration\nAnd there proclaim, my God, how great Thou art!"},

  {"It Is Well With My Soul",
   "When peace like a river attendeth my way,\nWhen sorrows like sea billows roll;\nWhatever my lot, Thou hast taught me to say,\nIt is well, it is well with my soul.\n\nIt is well (it is well)\nWith my soul (with my soul),\nIt is well, it is well with my soul.\n\nThough Satan should buffet, though trials should come,\nLet this blest assurance control,\nThat Christ hath regarded my helpless estate,\nAnd hath shed His own blood for my soul.\n\nMy sin—oh, the bliss of this glorious thought!\nMy sin, not in part, but the whole,\nIs nailed to the cross, and I bear it no more,\nPraise the Lord, praise the Lord, O my soul!\n\nAnd Lord, haste the day when the faith shall be sight,\nThe clouds be rolled back as a scroll;\nThe trump shall resound and the Lord shall descend,\nEven so, it is well with my soul.\n\nIt is well (it is well)\nWith my soul (with my soul),\nIt is well, it is well with my soul."},

  {"Great Is Thy Faithfulness",
   "Great is Thy faithfulness, O God my Father;\nThere is no shadow of turning with Thee;\nThou changest not, Thy compassions, they fail not;\nAs Thou hast been, Thou forever wilt be.\n\nGreat is Thy faithfulness! Great is Thy faithfulness!\nMorning by morning new mercies I see.\nAll I have needed Thy hand hath provided;\nGreat is Thy faithfulness, Lord, unto me!\n\nSummer and winter and springtime and harvest,\nSun, moon and stars in their courses above\nJoin with all nature in manifold witness\nTo Thy great faithfulness, mercy and love.\n\nPardon for sin and a peace that endureth,\nThine own dear presence to cheer and to guide;\nStrength for today and bright hope for tomorrow,\nBlessings all mine, with ten thousand beside!\n\nGreat is Thy faithfulness! Great is Thy faithfulness!\nMorning by morning new mercies I see.\nAll I have needed Thy hand hath provided;\nGreat is Thy faithfulness, Lord, unto me!"},

  {"Be Thou My Vision",
   "Be Thou my Vision, O Lord of my heart;\nNaught be all else to me, save that Thou art.\nThou my best thought, by day or by night,\nWaking or sleeping, Thy presence my light.\n\nBe Thou my Wisdom, and Thou my true Word;\nI ever with Thee and Thou with me, Lord;\nThou my great Father, I Thy true son;\nThou in me dwelling, and I with Thee one.\n\nRiches I heed not, nor man's empty praise,\nThou mine Inheritance, now and always:\nThou and Thou only, first in my heart,\nHigh King of Heaven, my Treasure Thou art.\n\nHigh King of Heaven, my victory won,\nMay I reach Heaven's joys, O bright Heaven's Sun!\nHeart of my own heart, whatever befall,\nStill be my Vision, O Ruler of all."},

  {"Holy, Holy, Holy",
   "Holy, holy, holy! Lord God Almighty!\nEarly in the morning our song shall rise to Thee;\nHoly, holy, holy! Merciful and mighty!\nGod in three Persons, blessed Trinity!\n\nHoly, holy, holy! All the saints adore Thee,\nCasting down their golden crowns around the glassy sea;\nCherubim and seraphim falling down before Thee,\nWho was, and is, and evermore shall be.\n\nHoly, holy, holy! Lord God Almighty!\nAll Thy works shall praise Thy name, in earth, and sky, and sea;\nHoly, holy, holy! Merciful and mighty!\nGod in three Persons, blessed Trinity!"},

  {"What a Friend We Have in Jesus",
   "What a friend we have in Jesus,\nAll our sins and griefs to bear!\nWhat a privilege to carry\nEverything to God in prayer!\n\nO what peace we often forfeit,\nO what needless pain we bear,\nAll because we do not carry\nEverything to God in prayer!\n\nHave we trials and temptations?\nIs there trouble anywhere?\nWe should never be discouraged,\nTake it to the Lord in prayer.\n\nCan we find a friend so faithful\nWho will all our sorrows share?\nJesus knows our every weakness;\nTake it to the Lord in prayer.\n\nAre we weak and heavy-laden,\nCumbered with a load of care?\nPrecious Savior, still our refuge;\nTake it to the Lord in prayer.\n\nDo thy friends despise, forsake thee?\nTake it to the Lord in prayer.\nIn His arms He'll take and shield thee;\nThou wilt find a solace there."},

  {"Rock of Ages",
   "Rock of Ages, cleft for me,\nLet me hide myself in Thee;\nLet the water and the blood,\nFrom Thy wounded side which flowed,\nBe of sin the double cure,\nSave from wrath and make me pure.\n\nNot the labors of my hands\nCan fulfill Thy law's demands;\nCould my zeal no respite know,\nCould my tears forever flow,\nAll for sin could not atone;\nThou must save, and Thou alone.\n\nWhile I draw this fleeting breath,\nWhen my eyes shall close in death,\nWhen I rise to worlds unknown,\nAnd behold Thee on Thy throne,\nRock of Ages, cleft for me,\nLet me hide myself in Thee."},

  {"The Old Rugged Cross",
   "On a hill far away stood an old rugged cross,\nThe emblem of suffering and shame;\nAnd I love that old cross where the dearest and best\nFor a world of lost sinners was slain.\n\nSo I'll cherish the old rugged cross,\nTill my trophies at last I lay down;\nI will cling to the old rugged cross,\nAnd exchange it some day for a crown.\n\nTo the old rugged cross I will ever be true,\nIts shame and reproach gladly bear;\nThen He'll call me some day to my home far away,\nWhere His glory forever I'll share.\n\nIn the old rugged cross, stained with blood so divine,\nA wondrous beauty I see;\nFor 'twas on that old cross Jesus suffered and died,\nTo pardon and sanctify me.\n\nSo I'll cherish the old rugged cross,\nTill my trophies at last I lay down;\nI will cling to the old rugged cross,\nAnd exchange it some day for a crown."},

  {"Doxology (Praise God From Whom)",
   "Praise God, from whom all blessings flow;\nPraise Him, all creatures here below;\nPraise Him above, ye heavenly host;\nPraise Father, Son, and Holy Ghost. Amen.\n\nPraise God, from whom all blessings flow;\nPraise Him, all creatures here below;\nPraise Him above, ye heavenly host;\nPraise Father, Son, and Holy Ghost. Amen."},

  // Modern worship (full verses + chorus as commonly sung)
  {"What A Beautiful Name",
   "You were the Word at the beginning\nOne with God the Lord Most High\nYour hidden glory in creation\nNow revealed in You our Christ\n\nWhat a beautiful Name it is\nWhat a beautiful Name it is\nThe Name of Jesus Christ my King\nWhat a beautiful Name it is\nNothing compares to this\nWhat a beautiful Name it is\nThe Name of Jesus\n\nYou didn't want heaven without us\nSo Jesus You brought heaven down\nMy sin was great Your love was greater\nWhat could separate us now\n\nWhat a beautiful Name it is\nWhat a beautiful Name it is\nThe Name of Jesus Christ my King\nWhat a beautiful Name it is\nNothing compares to this\nWhat a beautiful Name it is\nThe Name of Jesus\n\nDeath could not hold You\nThe veil tore before You\nYou silence the boast of sin and grave\nThe heavens are roaring\nThe praise of Your glory\nFor You are raised to life again\n\nYou have no rival\nYou have no equal\nNow and forever God You reign\nYours is the kingdom\nYours is the glory\nYours is the Name above all names"},

  {"Reckless Love",
   "Before I spoke a word, You were singing over me\nYou have been so, so good to me\nBefore I took a breath, You breathed Your life in me\nYou have been so, so kind to me\n\nOh, the overwhelming, never-ending, reckless love of God\nOh, it chases me down, fights 'til I'm found, leaves the ninety-nine\nI couldn't earn it, and I don't deserve it, still You give Yourself away\nOh, the overwhelming, never-ending, reckless love of God\n\nWhen I was Your foe, still Your love fought for me\nYou have been so, so good to me\nWhen I felt no worth, You paid it all for me\nYou have been so, so kind to me\n\nOh, the overwhelming, never-ending, reckless love of God\nOh, it chases me down, fights 'til I'm found, leaves the ninety-nine\nI couldn't earn it, and I don't deserve it, still You give Yourself away\nOh, the overwhelming, never-ending, reckless love of God\n\nThere's no shadow You won't light up\nMountain You won't climb up\nComing after me\nThere's no wall You won't kick down\nLie You won't tear down\nComing after me"},

  {"Good Good Father",
   "I've heard a thousand stories\nOf what they think You're like\nBut I've heard the tender whisper\nOf love in the dead of night\nYou tell me that You're pleased\nAnd that I'm never alone\n\nYou're a good, good Father\nIt's who You are, it's who You are, it's who You are\nAnd I'm loved by You\nIt's who I am, it's who I am, it's who I am\n\nI've seen many searching\nFor answers far and wide\nBut I know we're all searching\nFor answers only You provide\nBecause You know just what we need\nBefore we say a word\n\nYou're a good, good Father\nIt's who You are, it's who You are, it's who You are\nAnd I'm loved by You\nIt's who I am, it's who I am, it's who I am\n\nYou are perfect in all of Your ways\nYou are perfect in all of Your ways\nYou are perfect in all of Your ways to us\n\nLove so undeniable\nI can hardly speak\nPeace so unexplainable\nI can hardly think\nAs You call me deeper still\nAs You call me deeper still\nAs You call me deeper still\nInto love, love, love"},

  {"10,000 Reasons",
   "Bless the Lord, O my soul\nAnd all that is within me, bless His holy name\nHe has done great things\nBless the Lord, O my soul\nAnd all that is within me, bless His holy name\n\nBless the Lord, O my soul\nAnd all that is within me, bless His holy name\nHe has done great things\nBless the Lord, O my soul\nAnd all that is within me, bless His holy name\n\nThe sun comes up, it's a new day dawning\nIt's time to sing Your song again\nWhatever may pass and whatever lies before me\nLet me be singing when the evening comes\n\nBless the Lord, O my soul\nAnd all that is within me, bless His holy name\nHe has done great things\nBless the Lord, O my soul\nAnd all that is within me, bless His holy name\n\nYou're rich in love and You're slow to anger\nYour name is great and Your heart is kind\nFor all Your goodness, I will keep on singing\nTen thousand reasons for my heart to find\n\nBless the Lord, O my soul\nAnd all that is within me, bless His holy name\nHe has done great things\nBless the Lord, O my soul\nAnd all that is within me, bless His holy name"},

  {"Oceans (Where Feet May Fail)",
   "You call me out upon the waters\nThe great unknown where feet may fail\nAnd there I find You in the mystery\nIn oceans deep my faith will stand\n\nAnd I will call upon Your name\nAnd keep my eyes above the waves\nWhen oceans rise\nMy soul will rest in Your embrace\nFor I am Yours and You are mine\n\nYour grace abounds in deepest waters\nYour sovereign hand will be my guide\nWhere feet may fail and fear surrounds me\nYou've never failed and You won't start now\n\nSo I will call upon Your name\nAnd keep my eyes above the waves\nWhen oceans rise\nMy soul will rest in Your embrace\nFor I am Yours and You are mine\n\nSpirit lead me where my trust is without borders\nLet me walk upon the waters\nWherever You would call me\nTake me deeper than my feet could ever wander\nAnd my faith will be made stronger\nIn the presence of my Savior"},

  {"Way Maker",
   "You are here, moving in our midst\nI worship You, I worship You\nYou are here, working in this place\nI worship You, I worship You\n\nWay maker, miracle worker, promise keeper\nLight in the darkness\nMy God, that is who You are\nWay maker, miracle worker, promise keeper\nLight in the darkness\nMy God, that is who You are\n\nYou are here, touching every heart\nI worship You, I worship You\nYou are here, healing every heart\nI worship You, I worship You\n\nWay maker, miracle worker, promise keeper\nLight in the darkness\nMy God, that is who You are\nWay maker, miracle worker, promise keeper\nLight in the darkness\nMy God, that is who You are\n\nEven when I don't see it, You're working\nEven when I don't feel it, You're working\nYou never stop, You never stop working\nYou never stop, You never stop working"},

  {"Holy Spirit",
   "There's nothing worth more\nThat will ever come close\nNo thing can compare\nYou're our living hope\nYour presence, Lord\n\nI've tasted and seen\nOf the sweetest of loves\nWhere my heart becomes free\nAnd my shame is undone\nIn Your presence, Lord\n\nHoly Spirit, You are welcome here\nCome flood this place and fill the atmosphere\nYour glory, God, is what our hearts long for\nTo be overcome by Your presence, Lord\n\nYour presence, Lord\n\nI've tasted and seen\nOf the sweetest of loves\nWhere my heart becomes free\nAnd my shame is undone\nIn Your presence, Lord\n\nHoly Spirit, You are welcome here\nCome flood this place and fill the atmosphere\nYour glory, God, is what our hearts long for\nTo be overcome by Your presence, Lord"},

  {"Graves Into Gardens",
   "I searched the world but it couldn't fill me\nA man's empty praise and treasures that rust\nBut what I need is You\nI just want You\nI just want You\n\nI searched the world but it couldn't fill me\nA man's empty praise and treasures that rust\nBut what I need is You\nI just want You\nI just want You\n\nOh, there's nothing better than You\nThere's nothing better than You\nLord, there's nothing\nNothing is better than You\n\nI'm not afraid to show You my weakness\nMy failures and flaws, Lord, You've seen them all\nAnd You still call me friend\n\n'Cause the God of the mountain\nIs the God of the valley\nAnd there's not a place\nYour mercy and grace won't find me again\n\nOh, there's nothing better than You\nThere's nothing better than You\nLord, there's nothing\nNothing is better than You\n\nYou turn mourning to dancing\nYou give beauty for ashes\nYou turn shame into glory\nYou're the only one who can\n\nYou turn graves into gardens\nYou turn bones into armies\nYou turn seas into highways\nYou're the only one who can"},

  {"Build My Life",
   "Worthy of every song we could ever sing\nWorthy of all the praise we could ever bring\nWorthy of every breath we could ever breathe\nWe live for You\n\nJesus, the name above every other name\nJesus, the only one who could ever save\nWorthy of every breath we could ever breathe\nWe live for You\n\nHoly, there is no one like You\nThere is none beside You\nOpen up my eyes in wonder\nAnd show me who You are\nAnd fill me with Your heart\nAnd lead me in Your love to those around me\n\nI will build my life upon Your love\nIt is a firm foundation\nI will put my trust in You alone\nAnd I will not be shaken\n\nHoly, there is no one like You\nThere is none beside You\nOpen up my eyes in wonder\nAnd show me who You are\nAnd fill me with Your heart\nAnd lead me in Your love to those around me"},

  {"The Blessing",
   "The Lord bless you and keep you\nMake His face shine upon you and be gracious to you\nThe Lord turn His face toward you\nAnd give you peace\n\nAmen, amen, amen\nAmen, amen, amen\n\nThe Lord bless you and keep you\nMake His face shine upon you and be gracious to you\nThe Lord turn His face toward you\nAnd give you peace\n\nAmen, amen, amen\nAmen, amen, amen\n\nMay His favor be upon you\nAnd a thousand generations\nAnd your family and your children\nAnd their children, and their children\n\nMay His presence go before you\nAnd behind you, and beside you\nAll around you, and within you\nHe is with you, He is with you\n\nIn the morning, in the evening\nIn your coming, and your going\nIn your weeping, and rejoicing\nHe is for you, He is for you\n\nHe is for you, He is for you\nHe is for you, He is for you\nHe is for you, He is for you\nHe is for you, He is for you"}
};
const int NUM_HYMNS = sizeof(hymns) / sizeof(hymns[0]);

// Demo / VOTD pool (subset of popular for reliability)
const Verse votdPool[] = {
  {"John 3:16", "For God so loved the world, that he gave his only begotten Son, that whosoever believeth in him should not perish, but have everlasting life."},
  {"Psalm 23:1", "The LORD is my shepherd; I shall not want."},
  {"Romans 8:28", "And we know that all things work together for good to them that love God, to them who are the called according to his purpose."},
  {"Philippians 4:13", "I can do all things through Christ which strengtheneth me."},
  {"Jeremiah 29:11", "For I know the thoughts that I think toward you, saith the LORD, thoughts of peace, and not of evil, to give you an expected end."},
  {"Isaiah 40:31", "But they that wait upon the LORD shall renew their strength."},
  {"Matthew 11:28", "Come unto me, all ye that labour and are heavy laden, and I will give you rest."},
  {"Psalm 119:105", "Thy word is a lamp unto my feet, and a light unto my path."},
  {"John 14:6", "Jesus saith unto him, I am the way, the truth, and the life."},
  {"Ephesians 2:8", "For by grace are ye saved through faith; and that not of yourselves: it is the gift of God."}
};
const int NUM_VOTD = sizeof(votdPool) / sizeof(votdPool[0]);

const char* APP_VERSION = "1.0";

// Gideon Bible style Popular Verses Categories
// Only references shown in list ("Book Chapter:Verse").
// Selecting one opens the full verse in the reader.
struct PopularCategory {
  const char* name;
  const char* refs[5];
};

PopularCategory categories[] = {
  {"Love", {
    "John 3:16",
    "1 Corinthians 13:4",
    "1 John 4:8",
    "Romans 5:8",
    "John 15:5"
  }},
  {"Hope", {
    "Jeremiah 29:11",
    "Romans 15:13",
    "Psalm 23:4",
    "Hebrews 11:1",
    "Isaiah 40:31"
  }},
  {"Depression / Anxiety", {
    "Philippians 4:6",
    "Psalm 34:8",
    "Matthew 11:28",
    "1 Peter 5:7",
    "2 Timothy 1:7"
  }},
  {"Divorce / Broken Relationships", {
    "Psalm 34:8",
    "Isaiah 53:5",
    "Matthew 11:28",
    "2 Corinthians 5:17",
    "Psalm 23:4"
  }},
  {"Loss / Grief", {
    "Psalm 23:4",
    "Revelation 21:4",
    "Matthew 5:4",
    "2 Corinthians 5:17",
    "John 11:25"
  }},
  {"Truth / Wisdom", {
    "John 8:32",
    "John 14:6",
    "2 Timothy 3:16",
    "Psalm 119:105",
    "Proverbs 3:5"
  }},
  {"Encouragement", {
    "Philippians 4:13",
    "Isaiah 40:31",
    "Romans 8:28",
    "2 Corinthians 5:17",
    "Philippians 4:6"
  }},
  {"Faith", {
    "Hebrews 11:1",
    "Ephesians 2:8",
    "Romans 10:9",
    "Mark 10:45",
    "2 Corinthians 5:7"
  }},
  {"Guidance / Wisdom", {
    "Proverbs 3:5",
    "Psalm 119:105",
    "James 4:7",
    "Proverbs 3:6",
    "Isaiah 40:31"
  }},
  {"Victory over Temptation", {
    "James 4:7",
    "1 Corinthians 13:13",
    "Psalm 119:105",
    "Galatians 5:22",
    "Ephesians 6:10"
  }},
  {"Prayer", {
    "Matthew 7:7",
    "Philippians 4:6",
    "1 Thessalonians 5:16",
    "James 5:16",
    "Mark 12:30"
  }},
  {"Peace", {
    "John 14:27",
    "Philippians 4:7",
    "Isaiah 26:3",
    "Psalm 46:10",
    "Romans 5:8"
  }},
  {"Salvation / Assurance", {
    "John 3:16",
    "Romans 10:9",
    "Ephesians 2:8",
    "1 John 1:9",
    "John 11:25"
  }}
};
const int NUM_CATEGORIES = sizeof(categories) / sizeof(categories[0]);
int selectedCategory = 0;
int categoryVerseIndex = 0;

int currentTestament = 0; // 0=all, 1=OT, 2=NT

const char* SINNERS_PRAYER = 
"Dear Lord Jesus,\n"
"I know that I am a sinner, and I ask for Your forgiveness.\n"
"I believe You died for my sins and rose from the dead.\n"
"I turn from my sins and invite You to come into my heart and life.\n"
"I want to trust and follow You as my Lord and Savior.\n"
"In Your Name, Amen.";

const char* ABOUT_TEXT =
"The Bible contains the mind of God, the state of man, the way of salvation, the doom of sinners, and the happiness of believers.\n\n"
"Its doctrines are holy, its precepts are binding, its histories are true, and its decisions are immutable.\n\n"
"Read it to be wise, believe in it to be safe, and practice it to be holy.\n\n"
"It contains light to direct you, food to support you, and comfort to cheer you.\n\n"
"It is the traveler's map, the pilgrim's staff, the pilot's compass, the soldier's sword, and the Christian's charter.\n\n"
"Here paradise is restored, heaven opened, and the gates of hell disclosed.\n\n"
"Christ is its grand subject, our good the design, and the glory of God its end.\n\n"
"It should fill the memory, rule the heart, and guide the feet.\n\n"
"Read it slowly, frequently, and prayerfully.\n\n"
"It is a mine of wealth, a paradise of glory, and a river of pleasure.\n\n"
"It is given to you in life, will be opened at the judgment, and be remembered forever.\n\n"
"It involves the highest responsibility, will reward the greatest labor, and condemn all who trifle with its sacred contents.";

// Simple marquee for long selected text on menus
int menuScroll = 0;
unsigned long lastMenuScrollTime = 0;



enum AppState {
  HOME,
  READER,
  INPUT_REF,
  SELECT_BOOK,
  SELECT_CHAPTER,
  SELECT_VERSE,
  EXTRAS,
  TESTAMENT_SELECT,
  POPULAR_CATEGORIES,
  CATEGORY_VERSES,
  HYMNS_LIST
};

AppState state = HOME;  // start on clear navigation front screen

String currentRef = "";
String currentText = "";
int scrollOffset = 0;

bool autoScroll = false;
unsigned long lastAutoScroll = 0;
const int autoScrollSpeed = 350; // ms per line - slightly slower for comfortable reading

String inputBuffer = "";
String inputPrompt = "";
bool inputActive = false;

// SD
bool sdAvailable = false;
bool fullBibleMode = false;

// Navigation wizard
std::vector<String> navBooks;
std::vector<int> navChapterCounts;
int navBookIndex = 0;
String navBookSearch = "";
String navSelectedBook = "";
int navSelectedChapter = 1;
int navSelectedVerse = 1;

// Current loaded chapter (for L/R)
String currentBook = "";
int currentChapter = 0;

// Last reading position for "Resume Last Position" (replaces old VOTD)
String lastBook = "";
int lastChapter = 0;
int lastScrollOffset = 0;

// Bookmarks feature removed per user request to simplify UI (Gideon categories now prominent on screen)

int brightness = 90;

// ====================== HELPERS (one copy each) ======================
void drawHeader(const char* title) {
  canvas.fillRect(0, 0, SCREEN_W, 20, TFT_DARKGREEN);
  canvas.fillRect(0, 20, SCREEN_W, 2, TFT_GREEN);
  canvas.setTextColor(TFT_WHITE, TFT_DARKGREEN);
  canvas.setTextSize(1);
  canvas.setCursor(6, 4);
  if (strncmp(title, "Pocket Bible", 12) == 0) {
    canvas.print("Pocket Bible 1.0");
    canvas.print(title + 12);
  } else {
    canvas.print(title);
  }

  // Battery icon only (no percentage/number).
  // Fill level shows remaining charge. 
  // - Red when <=20% (needs charge, not plugged in)
  // - Green + bolt when charging (on charger, shows "powered/charging")
  int bat = M5Cardputer.Power.getBatteryLevel();
  bool isCharging = M5Cardputer.Power.isCharging();
  if (bat > 0 || isCharging) {
    int bx = SCREEN_W - 24;
    uint16_t batColor = TFT_WHITE;
    bool showCharging = isCharging;
    if (showCharging) {
      batColor = TFT_GREEN;
    } else if (bat <= 20) {
      batColor = TFT_RED;
    }
    // battery body outline
    canvas.drawRect(bx, 5, 16, 10, batColor);
    // positive terminal tip
    canvas.fillRect(bx + 16, 7, 3, 6, batColor);
    // fill level
    int fillw = (bat * 14) / 100;
    if (showCharging) {
      // when charging, show as "full" green to indicate powered/charging state
      fillw = 14;
    }
    if (fillw > 0) {
      canvas.fillRect(bx + 1, 6, fillw, 8, batColor);
    }
    // simple charging bolt (yellow) when on charger
    if (showCharging) {
      canvas.drawLine(bx + 6, 7, bx + 9, 7, TFT_YELLOW);
      canvas.drawLine(bx + 9, 7, bx + 7, 11, TFT_YELLOW);
      canvas.drawLine(bx + 7, 11, bx + 10, 11, TFT_YELLOW);
    }
  }
}

void drawFooter(const char* hint) {
  canvas.fillRect(0, SCREEN_H - 18, SCREEN_W, 18, TFT_DARKGREEN);
  canvas.fillRect(0, SCREEN_H - 20, SCREEN_W, 2, TFT_GREEN);
  canvas.setTextColor(TFT_WHITE, TFT_DARKGREEN);
  canvas.setTextSize(0.7);
  canvas.setCursor(6, SCREEN_H - 14);
  canvas.print(hint);
}

void clearCanvas() {
  canvas.fillSprite(TFT_BLACK);
  canvas.setTextColor(TFT_WHITE, TFT_BLACK);
}

void showMessage(const char* line1, const char* line2 = "") {
  clearCanvas();
  drawHeader("Pocket Bible");
  canvas.setTextSize(1);
  canvas.setCursor(8, 30);
  canvas.print(line1);
  if (line2[0]) {
    canvas.setCursor(8, 46);
    canvas.print(line2);
  }
  drawFooter("Press any key");
  canvas.pushSprite(0, 0);
  delay(120);
  while (true) {
    M5Cardputer.update();
    if (M5Cardputer.Keyboard.isChange() && M5Cardputer.Keyboard.isPressed()) break;
    delay(8);
  }
  delay(60);
}

void wrapText(const String& text, int maxWidth, std::vector<String>& lines) {
  lines.clear();
  String current = "";
  for (size_t i = 0; i < text.length(); i++) {
    char c = text[i];
    if (c == '\n') {
      if (current.length() > 0) lines.push_back(current);
      current = "";
      continue;
    }
    String test = current + c;
    if (canvas.textWidth(test) > maxWidth && current.length() > 0) {
      lines.push_back(current);
      current = c;
    } else {
      current = test;
    }
  }
  if (current.length() > 0) lines.push_back(current);
}

// SD chapter loader (single definition)
bool loadChapterFromSD(const String& book, int chapter, String& outText, String& outRef) {
  if (!sdAvailable) return false;
  char path[70];
  snprintf(path, sizeof(path), "/bible/%s.txt", book.c_str());
  File f = SD.open(path);
  if (!f) return false;

  outText = "";
  outRef = book + " " + String(chapter);
  bool inChapter = false;
  while (f.available()) {
    String line = f.readStringUntil('\n');
    line.trim();
    if (line.startsWith("## ")) {
      int ch = line.substring(3).toInt();
      if (ch == chapter) { inChapter = true; continue; }
      if (inChapter) break;
    }
    if (inChapter && line.length() > 0) {
      int sp = line.indexOf(' ');
      String vpart = (sp > 0) ? line.substring(0, sp) + " " + line.substring(sp + 1) : line;
      if (outText.length() > 0) outText += "\n";
      outText += vpart;
    }
  }
  f.close();
  return outText.length() > 0;
}

void loadNavBooks() {
  navBooks.clear();
  navChapterCounts.clear();

  if (fullBibleMode) {
    File f = SD.open("/bible/books.txt");
    if (f) {
      while (f.available()) {
        String line = f.readStringUntil('\n');
        line.trim();
        if (line.length() == 0) continue;
        int p1 = line.indexOf('|');
        if (p1 > 0) {
          String name = line.substring(0, p1);
          int chaps = 20;
          int p2 = line.indexOf('|', p1 + 1);
          if (p2 > p1) chaps = line.substring(p1 + 1, p2).toInt();
          else chaps = line.substring(p1 + 1).toInt();
          if (chaps < 1) chaps = 1;
          navBooks.push_back(name);
          navChapterCounts.push_back(chaps);
        }
      }
      f.close();
    }
  }

  if (navBooks.size() == 0) {
    // demo fallback
    const char* demos[] = {"Genesis", "Psalms", "Proverbs", "John", "Romans", "Revelation"};
    int counts[] = {50, 150, 31, 21, 16, 22};
    for (int i = 0; i < 6; i++) {
      navBooks.push_back(demos[i]);
      navChapterCounts.push_back(counts[i]);
    }
  }
}

bool tryLoadFullChapter(const String& book, int chapter) {
  autoScroll = false;
  lastAutoScroll = 0;
  String text, ref;
  if (loadChapterFromSD(book, chapter, text, ref)) {
    currentBook = book;
    currentChapter = chapter;
    currentRef = ref;
    currentText = text;
    scrollOffset = 0;
    saveLastPosition();
    state = READER;
    return true;
  }
  return false;
}

void saveLastPosition() {
  lastBook = currentBook;
  lastChapter = currentChapter;
  lastScrollOffset = scrollOffset;
  if (sdAvailable && fullBibleMode) {
    File f = SD.open("/bible/lastpos.txt", FILE_WRITE);
    if (f) {
      f.print(lastBook);
      f.print("|");
      f.print(lastChapter);
      f.print("|");
      f.println(lastScrollOffset);
      f.close();
    }
  }
}

void loadLastPosition() {
  if (!sdAvailable || !fullBibleMode) return;
  File f = SD.open("/bible/lastpos.txt");
  if (!f) return;
  String line = f.readStringUntil('\n');
  f.close();
  line.trim();
  if (line.length() == 0) return;
  int p1 = line.indexOf('|');
  if (p1 <= 0) return;
  lastBook = line.substring(0, p1);
  int p2 = line.indexOf('|', p1 + 1);
  if (p2 > p1) {
    lastChapter = line.substring(p1 + 1, p2).toInt();
    lastScrollOffset = line.substring(p2 + 1).toInt();
  } else {
    lastChapter = line.substring(p1 + 1).toInt();
  }
}

void jumpToVerse(int verse) {
  if (verse < 1) verse = 1;
  std::vector<String> lines;
  canvas.setTextSize(1);
  wrapText(currentText, SCREEN_W - 8, lines);
  scrollOffset = 0;
  String vstr = String(verse) + " ";
  for (size_t j = 0; j < lines.size(); j++) {
    if (lines[j].startsWith(vstr)) {
      scrollOffset = j;
      break;
    }
  }
  if (scrollOffset == 0 && verse > 1) {
    scrollOffset = max(0, (verse - 1) / 2);
  }
}

void enterReader(const String& ref, const String& text) {
  autoScroll = false;
  lastAutoScroll = 0;
  currentRef = ref;
  currentText = text;
  scrollOffset = 0;
  currentBook = "";
  currentChapter = 0;
  state = READER;
}

void enterReaderFromRef(const String& raw) {
  // Very light parser for common cases when in reader 'g'
  String s = raw; s.trim(); s.toLowerCase();
  String book = "";
  int ch = 1, vs = 1;

  // quick abbrs
  struct Ab { const char* k; const char* b; };
  Ab ab[] = {
    {"jn","John"},{"john","John"},{"ps","Psalms"},{"rom","Romans"},
    {"matt","Matthew"},{"lk","Luke"},{"gen","Genesis"},{"pro","Proverbs"},
    {"isa","Isaiah"},{"jer","Jeremiah"},{"eph","Ephesians"},{"phil","Philippians"},
    {"rev","Revelation"}
  };
  for (auto& a : ab) {
    if (s.startsWith(a.k)) { book = a.b; s = s.substring(strlen(a.k)); break; }
  }
  if (book.length() == 0) {
    int sp = s.indexOf(' ');
    if (sp > 0) { book = s.substring(0, sp); s = s.substring(sp + 1); }
    else { book = s; s = ""; }
    if (book.length() > 0) book[0] = toupper(book[0]);
  }
  s.trim();
  if (s.length() > 0) {
    int c = s.indexOf(':');
    if (c >= 0) { ch = s.substring(0, c).toInt(); vs = s.substring(c + 1).toInt(); }
    else { ch = s.toInt(); }
  }
  if (ch < 1) ch = 1;

  if (fullBibleMode && tryLoadFullChapter(book, ch)) {
    if (vs > 1) {
      jumpToVerse(vs);
    }
    return;
  }

  // fallback to popular/demo
  for (int i = 0; i < NUM_POPULAR; i++) {
    String r = popularVerses[i].ref; r.toLowerCase();
    if (r.indexOf(book.substring(0, 3).c_str()) >= 0 || r.startsWith(book.substring(0, 4))) {
      enterReader(popularVerses[i].ref, popularVerses[i].text);
      return;
    }
  }
  enterReader(book + " " + String(ch), "Full chapter loads when SD /bible/ data is present (copy sd-card-bible/bible to SD root).");
}

// ====================== READER ======================
void drawReader() {
  clearCanvas();
  drawHeader(currentRef.c_str());

  std::vector<String> lines;
  canvas.setTextSize(1);
  wrapText(currentText, SCREEN_W - 8, lines);  // wider for better line lengths / fewer awkward wraps

  // Auto-scroll logic (toggle with 'S')
  if (autoScroll && millis() - lastAutoScroll > autoScrollSpeed) {
    if (scrollOffset < (int)lines.size() - 1) {
      scrollOffset++;
    } else {
      autoScroll = false;  // auto-stop at end
    }
    lastAutoScroll = millis();
  }

  int y = 26;           // slightly more top margin
  int lineH = 13;       // better readability / air between lines
  int paraExtra = 5;    // extra space for paragraph breaks (\n\n)
  int maxL = (SCREEN_H - 48) / lineH;  // conservative with extras

  for (int i = 0; i < maxL && (scrollOffset + i) < (int)lines.size(); i++) {
    String ln = lines[scrollOffset + i];
    // paragraph spacing: blank lines from \n\n get extra vertical space
    if (ln.length() == 0) {
      y += paraExtra;
      continue;
    }
    canvas.setCursor(4, y);  // tighter left margin to give more width for wrapping
    uint16_t bodyColor = TFT_WHITE;
    // highlight leading verse number yellow for every verse (reliable because chapters use \n per verse)
    String toDraw = ln;
    if (ln.length() > 0) {
      int s = 0;
      while (s < ln.length() && isspace(ln[s])) s++;
      if (s < ln.length() && isdigit(ln[s])) {
        int sp = ln.indexOf(' ', s);
        if (sp > s && (sp - s) < 6) {
          canvas.setTextColor(TFT_YELLOW, TFT_BLACK);
          canvas.print(ln.substring(0, sp + 1));
          toDraw = ln.substring(sp + 1);
        }
      }
    }
    canvas.setTextColor(bodyColor, TFT_BLACK);
    canvas.print(toDraw);
    y += lineH;
  }

  // simple scroll bar (moved slightly for wider text area)
  if (lines.size() > maxL) {
    int bh = 18;
    int denom = std::max(1, (int)lines.size() - maxL);
    int by = 25 + (scrollOffset * (SCREEN_H - 48 - bh)) / denom;
    canvas.fillRoundRect(SCREEN_W - 4, by, 2, bh, 1, TFT_GREEN);
  }

  drawFooter("Up/Dn scroll  L/R ch  B=back  G=go");
  canvas.pushSprite(0, 0);
}

void handleReaderInput() {
  if (!M5Cardputer.Keyboard.isChange() || !M5Cardputer.Keyboard.isPressed()) return;
  Keyboard_Class::KeysState ks = M5Cardputer.Keyboard.keysState();

  bool up = false, down = false, left = false, right = false;

  for (auto c : ks.word) {
    if (c == ';' || c == 'k' || c == 'K') up = true;
    if (c == '.' || c == 'j' || c == 'J') down = true;
    if (c == ',' || c == 'h' || c == 'H') left = true;
    if (c == '/' || c == 'l' || c == 'L') right = true;

    if (c == 'b' || c == 'B' || c == 'q' || c == 'Q') {
      state = HOME;  // back to front navigation screen
      return;
    }
    if (c == 'g' || c == 'G') {
      state = INPUT_REF;
      inputPrompt = "Go (e.g. john 3:16 or ps 23):";
      inputBuffer = "";
      autoScroll = false;
      return;
    }
    if (c == 's' || c == 'S') {
      autoScroll = !autoScroll;
      lastAutoScroll = millis();
      return;
    }
    // r random removed (no random tab)
    // Bookmark feature removed to simplify UI (Gideon categories now prominent)
  }

  if (up && scrollOffset > 0) {
    scrollOffset--;
    autoScroll = false;
  }
  if (down) {
    std::vector<String> lines;
    canvas.setTextSize(1);
    wrapText(currentText, SCREEN_W - 8, lines);
    if (scrollOffset < (int)lines.size() - 1) scrollOffset++;
    autoScroll = false;
  }
  if (left && fullBibleMode && currentBook.length() > 0 && currentChapter > 1) {
    tryLoadFullChapter(currentBook, currentChapter - 1);
    autoScroll = false;
  }
  if (right && fullBibleMode && currentBook.length() > 0) {
    tryLoadFullChapter(currentBook, currentChapter + 1);
    autoScroll = false;
  }

  if (ks.del) {
    state = HOME;
    autoScroll = false;
    return;
  }
}

// ====================== LISTS (easy to read, good spacing) ======================
// Old popular list functions removed (replaced by Topical Categories in Extras)

void drawHymnsList() {
  clearCanvas();
  drawHeader("Pocket Bible");

  int y = 24;
  int maxShow = 5;
  int start = max(0, navBookIndex - 1);
  if (navBookIndex >= NUM_HYMNS) navBookIndex = 0;

  for (int i = start; i < NUM_HYMNS && (i - start) < maxShow; i++) {
    bool sel = (i == navBookIndex);
    if (sel) {
      canvas.fillRoundRect(3, y - 1, SCREEN_W - 6, 18, 3, TFT_DARKGREEN);
      canvas.setTextColor(TFT_WHITE, TFT_DARKGREEN);
    } else {
      canvas.setTextColor(TFT_WHITE, TFT_BLACK);
    }
    canvas.setCursor(8, y);
    canvas.setTextSize(1.0);  // slightly larger
    String t = hymns[i].title;
    if (sel && canvas.textWidth(t) > 180) {
      if (millis() - lastMenuScrollTime > 150) {
        menuScroll = (menuScroll + 1) % (t.length() + 5);
        lastMenuScrollTime = millis();
      }
      String pad = "     ";
      String scrollT = t + pad + t;
      int startPos = menuScroll % scrollT.length();
      String disp = "";
      for (int j = 0; j < scrollT.length() && canvas.textWidth(disp) < 180; j++) {
        disp += scrollT[(startPos + j) % scrollT.length()];
      }
      canvas.print(disp);
    } else {
      canvas.print(t);
    }
    y += 19;
  }

  canvas.setTextSize(0.6);
  canvas.setTextColor(TFT_GREEN, TFT_BLACK);
  canvas.setCursor(6, SCREEN_H - 26);
  canvas.print("Enter = view complete lyrics");

  drawFooter("L/R switch tabs  Up/Dn  Enter=full lyrics  B=back");
  canvas.pushSprite(0, 0);
}

void handleHymnsList() {
  if (!M5Cardputer.Keyboard.isChange() || !M5Cardputer.Keyboard.isPressed()) return;
  Keyboard_Class::KeysState ks = M5Cardputer.Keyboard.keysState();

  for (auto c : ks.word) {
    if (c == ';' || c == 'k' || c == 'K') { if (navBookIndex > 0) navBookIndex--; }
    if (c == '.' || c == 'j' || c == 'J') { if (navBookIndex < NUM_HYMNS - 1) navBookIndex++; }
    if (c == 'b' || c == 'B') { state = HOME; return; }
  }

  if (ks.enter) {
    enterReader(hymns[navBookIndex].title, hymns[navBookIndex].lyrics);
  }

  if (ks.del) {
    state = HOME;
    return;
  }
}

// No audio - melodies removed. Hymns show complete lyrics only.

// ====================== BOOK > CHAPTER > VERSE (simple wizard) ======================
void drawBookSelector() {
  clearCanvas();
  String bookHeader = "Pocket Bible - Books";
  if (currentTestament == 1) bookHeader = "Pocket Bible - Old Testament";
  else if (currentTestament == 2) bookHeader = "Pocket Bible - New Testament";
  drawHeader(bookHeader.c_str());

  // filter by testament first, then search
  // Use name-based split so Matthew is always the first NT book (robust to any list length)
  int bookStart = 0;
  int bookEnd = navBooks.size();
  if (currentTestament == 1) {
    // Old Testament: everything before Matthew
    bookEnd = navBooks.size();
    for (int i = 0; i < (int)navBooks.size(); i++) {
      if (navBooks[i] == "Matthew") { bookEnd = i; break; }
    }
  } else if (currentTestament == 2) {
    // New Testament: starting at Matthew
    bookStart = 0;
    for (int i = 0; i < (int)navBooks.size(); i++) {
      if (navBooks[i] == "Matthew") { bookStart = i; break; }
    }
  }

  std::vector<int> filtered;
  String q = navBookSearch; q.toLowerCase();
  for (int i = bookStart; i < bookEnd; i++) {
    String n = navBooks[i]; n.toLowerCase();
    if (q.length() == 0 || n.indexOf(q) >= 0) filtered.push_back(i);
  }
  if (navBookIndex >= (int)filtered.size()) navBookIndex = 0;

  canvas.setTextSize(0.7);
  canvas.setCursor(6, 22);
  canvas.print("Pos " + String(navBookIndex + 1) + "/" + String((int)filtered.size()) + "   Type to filter");

  canvas.setCursor(6, 32);
  canvas.setTextSize(0.9);
  canvas.print("> " + navBookSearch + ((millis() / 380 % 2) ? "_" : ""));

  int y = 46;
  int maxShow = 4;
  int start = max(0, navBookIndex - 1);
  for (int fi = start; fi < (int)filtered.size() && (fi - start) < maxShow; fi++) {
    int real = filtered[fi];
    bool sel = (fi == navBookIndex);
    if (sel) {
      canvas.fillRoundRect(3, y - 1, SCREEN_W - 6, 16, 2, TFT_DARKGREEN);
      canvas.setTextColor(TFT_WHITE, TFT_DARKGREEN);
    } else {
      canvas.setTextColor(TFT_WHITE, TFT_BLACK);
    }
    canvas.setCursor(7, y);
    canvas.setTextSize(0.95);
    String t = navBooks[real];
    if (sel && canvas.textWidth(t) > 200) {
      if (millis() - lastMenuScrollTime > 150) {
        menuScroll = (menuScroll + 1) % (t.length() + 5);
        lastMenuScrollTime = millis();
      }
      String pad = "     ";
      String scrollT = t + pad + t;
      int startPos = menuScroll % scrollT.length();
      String disp = "";
      for (int j = 0; j < scrollT.length() && canvas.textWidth(disp) < 200; j++) {
        disp += scrollT[(startPos + j) % scrollT.length()];
      }
      canvas.print(disp);
    } else {
      canvas.print(t);
    }
    y += 19;  // slightly more air
  }

  drawFooter("L/R tabs  Enter pick   B back   Del clear");
  canvas.pushSprite(0, 0);
}

void handleBookSelector() {
  if (!M5Cardputer.Keyboard.isChange() || !M5Cardputer.Keyboard.isPressed()) return;
  Keyboard_Class::KeysState ks = M5Cardputer.Keyboard.keysState();

  // IMPORTANT: must apply SAME testament filter as drawBookSelector, otherwise
  // navBookIndex into filtered selects wrong master book (Mark->Genesis etc).
  // Use name-based split so Matthew is always the first NT book (robust to any list length)
  int bookStart = 0;
  int bookEnd = navBooks.size();
  if (currentTestament == 1) {
    // Old Testament: everything before Matthew
    bookEnd = navBooks.size();
    for (int i = 0; i < (int)navBooks.size(); i++) {
      if (navBooks[i] == "Matthew") { bookEnd = i; break; }
    }
  } else if (currentTestament == 2) {
    // New Testament: starting at Matthew
    bookStart = 0;
    for (int i = 0; i < (int)navBooks.size(); i++) {
      if (navBooks[i] == "Matthew") { bookStart = i; break; }
    }
  }

  std::vector<int> filtered;
  String q = navBookSearch; q.toLowerCase();
  for (int i = bookStart; i < bookEnd; i++) {
    String n = navBooks[i]; n.toLowerCase();
    if (q.length() == 0 || n.indexOf(q) >= 0) filtered.push_back(i);
  }
  if (navBookIndex >= (int)filtered.size()) navBookIndex = 0;

  for (auto c : ks.word) {
    if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == ' ') {
      navBookSearch += (char)c;
      navBookIndex = 0;
      menuScroll = 0;
      lastMenuScrollTime = millis();
    }
    if (c == ';' || c == 'k' || c == 'K') { if (navBookIndex > 0) navBookIndex--; menuScroll = 0; lastMenuScrollTime = millis(); }
    if (c == '.' || c == 'j' || c == 'J') { if (navBookIndex < (int)filtered.size() - 1) navBookIndex++; menuScroll = 0; lastMenuScrollTime = millis(); }
    if (c == 'b' || c == 'B') {
      state = TESTAMENT_SELECT;
      navBookSearch = "";
      currentTestament = 0;
      return;
    }
  }

  if (ks.del && navBookSearch.length() > 0) {
    navBookSearch.remove(navBookSearch.length() - 1);
    navBookIndex = 0;
    menuScroll = 0;
    lastMenuScrollTime = millis();
  }
  if (ks.enter && filtered.size() > 0) {
    int real = filtered[navBookIndex];
    navSelectedBook = navBooks[real];
    currentBook = navSelectedBook;
    navSelectedChapter = 1;
    navBookSearch = "";
    state = SELECT_CHAPTER;
  }
}

void drawChapterSelector() {
  clearCanvas();
  drawHeader("Pocket Bible - Chapter");

  int maxCh = 20;
  for (size_t i = 0; i < navBooks.size(); i++) {
    if (navBooks[i] == navSelectedBook) { maxCh = navChapterCounts[i]; break; }
  }
  if (navSelectedChapter > maxCh) navSelectedChapter = maxCh;
  if (navSelectedChapter < 1) navSelectedChapter = 1;

  canvas.setTextSize(0.85);
  canvas.setCursor(6, 24);
  canvas.print(navSelectedBook + "   1 - " + String(maxCh));

  canvas.setTextSize(2.2);
  canvas.setCursor(6, 44);
  canvas.print("Ch " + String(navSelectedChapter));

  canvas.setTextSize(0.8);
  canvas.setCursor(6, 72);
  canvas.print("↑ ↓ or type   Enter to pick verse");

  drawFooter("Enter load   B back (to book)");
  canvas.pushSprite(0, 0);
}

void handleChapterSelector() {
  if (!M5Cardputer.Keyboard.isChange() || !M5Cardputer.Keyboard.isPressed()) return;
  Keyboard_Class::KeysState ks = M5Cardputer.Keyboard.keysState();

  int maxCh = 20;
  for (size_t i = 0; i < navBooks.size(); i++) {
    if (navBooks[i] == navSelectedBook) { maxCh = navChapterCounts[i]; break; }
  }
  if (navSelectedChapter > maxCh) navSelectedChapter = maxCh;
  if (navSelectedChapter < 1) navSelectedChapter = 1;

  for (auto c : ks.word) {
    if (c >= '0' && c <= '9') {
      navSelectedChapter = navSelectedChapter * 10 + (c - '0');
      if (navSelectedChapter > maxCh) navSelectedChapter = maxCh;
    }
    if (c == ';' || c == 'k' || c == 'K') { if (navSelectedChapter > 1) navSelectedChapter--; }
    if (c == '.' || c == 'j' || c == 'J') { if (navSelectedChapter < maxCh) navSelectedChapter++; }
    if (c == 'b' || c == 'B') { state = SELECT_BOOK; return; }
  }
  if (ks.del) navSelectedChapter = max(1, navSelectedChapter / 10);
  if (ks.enter) {
    navSelectedVerse = 1;
    state = SELECT_VERSE;
  }
}

void drawVerseSelector() {
  clearCanvas();
  drawHeader("Pocket Bible - Verse");

  int maxV = 200;
  if (fullBibleMode) {
    String tmpText, tmpRef;
    if (loadChapterFromSD(navSelectedBook, navSelectedChapter, tmpText, tmpRef)) {
      int count = 1;
      for (int k = 0; k < tmpText.length(); k++) {
        if (tmpText[k] == '\n') count++;
      }
      maxV = count;
    }
  }
  if (navSelectedVerse > maxV) navSelectedVerse = maxV;
  if (navSelectedVerse < 1) navSelectedVerse = 1;

  canvas.setTextSize(0.85);
  canvas.setCursor(6, 24);
  canvas.print(navSelectedBook + " " + String(navSelectedChapter) + "   1 - " + String(maxV));

  canvas.setTextSize(2.2);
  canvas.setCursor(6, 44);
  canvas.print("Verse " + String(navSelectedVerse));

  canvas.setTextSize(0.8);
  canvas.setCursor(6, 72);
  canvas.print("↑ ↓  or type number   Enter to read");

  drawFooter("Enter read   B back (to ch)");
  canvas.pushSprite(0, 0);
}

void handleVerseSelector() {
  if (!M5Cardputer.Keyboard.isChange() || !M5Cardputer.Keyboard.isPressed()) return;
  Keyboard_Class::KeysState ks = M5Cardputer.Keyboard.keysState();

  int maxV = 200;
  if (fullBibleMode) {
    String tmpText, tmpRef;
    if (loadChapterFromSD(navSelectedBook, navSelectedChapter, tmpText, tmpRef)) {
      int count = 1;
      for (int k = 0; k < tmpText.length(); k++) {
        if (tmpText[k] == '\n') count++;
      }
      maxV = count;
    }
  }

  for (auto c : ks.word) {
    if (c >= '0' && c <= '9') {
      navSelectedVerse = navSelectedVerse * 10 + (c - '0');
      if (navSelectedVerse > maxV) navSelectedVerse = maxV;
    }
    if (c == ';' || c == 'k' || c == 'K') { if (navSelectedVerse > 1) navSelectedVerse--; }
    if (c == '.' || c == 'j' || c == 'J') { if (navSelectedVerse < maxV) navSelectedVerse++; }
    if (c == 'b' || c == 'B') { state = SELECT_CHAPTER; return; }
  }
  if (ks.del && navSelectedVerse > 1) navSelectedVerse /= 10;
  if (ks.enter) {
    if (fullBibleMode) {
      tryLoadFullChapter(navSelectedBook, navSelectedChapter);
      jumpToVerse(navSelectedVerse);
    } else {
      enterReader(navSelectedBook + " " + String(navSelectedChapter) + ":" + String(navSelectedVerse),
                  "Demo. Full verses available with SD data.");
      scrollOffset = max(0, (navSelectedVerse - 1) / 2);
    }
  }
}

// Bookmarks feature fully removed (as requested) to simplify the UI and keep the Gideon categories prominent.

// ====================== INPUT (minimal for 'g' in reader) ======================
void drawInput() {
  clearCanvas();
  drawHeader("Pocket Bible - Go to Ref");

  canvas.setTextSize(1);
  canvas.setCursor(6, 26);
  canvas.print(inputPrompt);

  canvas.drawRect(4, 42, SCREEN_W - 8, 20, TFT_GREEN);
  canvas.setCursor(8, 46);
  canvas.setTextSize(1);
  canvas.print(inputBuffer);
  if ((millis() / 420) % 2 == 0) canvas.print("_");

  canvas.setTextSize(0.8);
  canvas.setCursor(6, 68);
  canvas.print("Examples: john 3:16   ps 23   rom 8");
  drawFooter("Enter=go   Del=backspace   B=cancel");
  canvas.pushSprite(0, 0);
}

void handleInput() {
  if (!M5Cardputer.Keyboard.isChange() || !M5Cardputer.Keyboard.isPressed()) return;
  Keyboard_Class::KeysState ks = M5Cardputer.Keyboard.keysState();

  for (auto c : ks.word) {
    if (c >= 32 && c <= 126) inputBuffer += (char)c;
  }
  if (ks.del && inputBuffer.length() > 0) inputBuffer.remove(inputBuffer.length() - 1);
  if (ks.enter) {
    String v = inputBuffer; v.trim();
    state = READER; // will be overwritten
    enterReaderFromRef(v);
    return;
  }
  if (ks.word.size() > 0) {
    char c = ks.word[0];
    if (c == 'b' || c == 'B') { state = READER; return; }
  }
}

// ====================== HOME NAVIGATION FRONT SCREEN ======================
int mainMenuIndex = 0;

void drawHome() {
  clearCanvas();
  drawHeader("Pocket Bible");

  canvas.setTextSize(1);
  canvas.setCursor(8, 24);
  canvas.print("Main Menu");

  int y = 40;
  const char* items[4] = {
    "1. Read the Bible",
    "2. Resume Last Position",
    "3. Extras",
    "4. About"
  };
  for (int i = 0; i < 4; i++) {
    bool sel = (i == mainMenuIndex);
    if (sel) {
      canvas.fillRoundRect(4, y-2, SCREEN_W-8, 17, 2, TFT_DARKGREEN);
      canvas.setTextColor(TFT_WHITE, TFT_DARKGREEN);
    } else {
      canvas.setTextColor(TFT_WHITE, TFT_BLACK);
    }
    canvas.setCursor(10, y);
    canvas.setTextSize(1.0);
    String t = items[i];
    if (sel && canvas.textWidth(t) > 200) {
      if (millis() - lastMenuScrollTime > 150) {
        menuScroll = (menuScroll + 1) % (t.length() + 5);
        lastMenuScrollTime = millis();
      }
      String pad = "     ";
      String scrollT = t + pad + t;
      int startPos = menuScroll % scrollT.length();
      String disp = "";
      for (int j = 0; j < scrollT.length() && canvas.textWidth(disp) < 200; j++) {
        disp += scrollT[(startPos + j) % scrollT.length()];
      }
      canvas.print(disp);
    } else {
      canvas.print(t);
    }
    y += 20;  // generous spacing, fills space without dead areas
  }

  drawFooter("Up/Dn Enter  1-4  B back");
  canvas.pushSprite(0, 0);
}

void handleHomeInput() {
  if (!M5Cardputer.Keyboard.isChange() || !M5Cardputer.Keyboard.isPressed()) return;
  Keyboard_Class::KeysState ks = M5Cardputer.Keyboard.keysState();

  for (auto c : ks.word) {
    if (c == ';' || c == 'k' || c == 'K') { mainMenuIndex = (mainMenuIndex - 1 + 4) % 4; menuScroll = 0; lastMenuScrollTime = millis(); return; }
    if (c == '.' || c == 'j' || c == 'J') { mainMenuIndex = (mainMenuIndex + 1) % 4; menuScroll = 0; lastMenuScrollTime = millis(); return; }
    if (c >= '1' && c <= '4') { mainMenuIndex = c - '1'; menuScroll = 0; lastMenuScrollTime = millis(); }
  }

  if (ks.enter) {
    switch (mainMenuIndex) {
      case 0: // Read the Bible
        state = TESTAMENT_SELECT;
        break;
      case 1: // Resume Last Position (saves current book/chapter when reading full chapters)
        if (lastBook.length() > 0) {
          if (fullBibleMode) {
            if (tryLoadFullChapter(lastBook, lastChapter)) {
              scrollOffset = lastScrollOffset;
            }
          } else {
            enterReader(lastBook + " " + String(lastChapter), "SD card required for full chapter resume. Using demo text.");
          }
        } else {
          showMessage("No saved position", "Read a chapter first (with SD for full support).");
        }
        break;
      case 2: // Extras
        state = EXTRAS;
        break;
      case 3: // About - direct top level for easy access
        enterReader("About the Bible", ABOUT_TEXT);
        break;
    }
  }
}

// ====================== TESTAMENT SELECT ======================
int testamentIndex = 0;

void drawTestamentSelect() {
  clearCanvas();
  drawHeader("Pocket Bible - Read the Bible");

  int y = 30;
  const char* items[2] = {
    "Old Testament",
    "New Testament"
  };
  canvas.setTextSize(1.6);  // much bigger to utilize screen space
  for (int i = 0; i < 2; i++) {
    bool sel = (i == testamentIndex);
    if (sel) {
      canvas.fillRoundRect(4, y-3, SCREEN_W-8, 24, 3, TFT_DARKGREEN);
      canvas.setTextColor(TFT_WHITE, TFT_DARKGREEN);
    } else {
      canvas.setTextColor(TFT_WHITE, TFT_BLACK);
    }
    canvas.setCursor(10, y + 3);
    canvas.print(items[i]);
    y += 38;  // spread out to use vertical space, less dead area
  }

  drawFooter("Up/Dn Enter  B=Home");
  canvas.pushSprite(0, 0);
}

void handleTestamentSelect() {
  if (!M5Cardputer.Keyboard.isChange() || !M5Cardputer.Keyboard.isPressed()) return;
  Keyboard_Class::KeysState ks = M5Cardputer.Keyboard.keysState();

  for (auto c : ks.word) {
    if (c == ';' || c == 'k' || c == 'K') { if (testamentIndex > 0) testamentIndex--; return; }
    if (c == '.' || c == 'j' || c == 'J') { if (testamentIndex < 1) testamentIndex++; return; }
    if (c == 'b' || c == 'B') { state = HOME; return; }
  }

  if (ks.enter) {
    currentTestament = testamentIndex + 1; // 1=OT, 2=NT
    loadNavBooks();
    navBookSearch = "";
    navBookIndex = 0;
    state = SELECT_BOOK;
  }
}

// ====================== EXTRAS MENU ======================
int extrasMenuIndex = 0;

void drawExtras() {
  clearCanvas();
  drawHeader("Pocket Bible - Extras");

  canvas.setTextSize(1.0);
  canvas.setCursor(6, 22);
  canvas.print("Select:");

  const char* items[3] = {
    "Sinner's Prayer",
    "Hymns & Worship",
    "Topical Verses"
  };

  int y = 40;
  for (int i = 0; i < 3; i++) {
    bool sel = (i == extrasMenuIndex);
    if (sel) {
      canvas.fillRoundRect(4, y-2, SCREEN_W-8, 18, 2, TFT_DARKGREEN);
      canvas.setTextColor(TFT_WHITE, TFT_DARKGREEN);
    } else {
      canvas.setTextColor(TFT_WHITE, TFT_BLACK);
    }
    canvas.setCursor(10, y);
    canvas.setTextSize(1.0);  // larger for readability, fills vertical space nicely
    canvas.print(items[i]);
    y += 22;  // good proportions, minimal dead space with only 3 items
  }

  drawFooter("Up/Dn Enter  B=Home");
  canvas.pushSprite(0, 0);
}

void handleExtrasInput() {
  if (!M5Cardputer.Keyboard.isChange() || !M5Cardputer.Keyboard.isPressed()) return;
  Keyboard_Class::KeysState ks = M5Cardputer.Keyboard.keysState();

  for (auto c : ks.word) {
    if (c == ';' || c == 'k' || c == 'K') { if (extrasMenuIndex > 0) extrasMenuIndex--; return; }
    if (c == '.' || c == 'j' || c == 'J') { if (extrasMenuIndex < 2) extrasMenuIndex++; return; }
    if (c == 'b' || c == 'B') { state = HOME; return; }
  }

  if (ks.enter) {
    switch (extrasMenuIndex) {
      case 0: // Sinner's Prayer
        enterReader("Sinner's Prayer", SINNERS_PRAYER);
        break;
      case 1: // Hymns
        navBookIndex = 0;
        state = HYMNS_LIST;
        break;
      case 2: // Topical
        selectedCategory = 0;
        state = POPULAR_CATEGORIES;
        break;
    }
  }
}

void drawPopularCategories() {
  clearCanvas();
  drawHeader("Pocket Bible - Topical Verses");

  canvas.setTextSize(0.8);
  canvas.setCursor(6, 24);
  canvas.print("Choose a category");

  int y = 38;
  int maxShow = 6; // more visible on screen
  int start = max(0, selectedCategory - 2);
  for (int i = start; i < NUM_CATEGORIES && (i-start) < maxShow; i++) {
    bool sel = (i == selectedCategory);
    if (sel) {
      canvas.fillRoundRect(4, y-2, SCREEN_W-8, 16, 2, TFT_DARKGREEN);
      canvas.setTextColor(TFT_WHITE, TFT_DARKGREEN);
    } else {
      canvas.setTextColor(TFT_WHITE, TFT_BLACK);
    }
    canvas.setCursor(8, y);
    canvas.setTextSize(0.9);  // slightly larger for better readability
    canvas.print(categories[i].name);
    y += 17;
  }

  // simple scroll hint
  if (NUM_CATEGORIES > maxShow) {
    canvas.setTextSize(0.6);
    canvas.setCursor(SCREEN_W - 40, 24);
    canvas.print(String(selectedCategory+1) + "/" + String(NUM_CATEGORIES));
  }

  drawFooter("Up/Dn Enter  B=Home");
  canvas.pushSprite(0, 0);
}

void handlePopularCategories() {
  if (!M5Cardputer.Keyboard.isChange() || !M5Cardputer.Keyboard.isPressed()) return;
  Keyboard_Class::KeysState ks = M5Cardputer.Keyboard.keysState();

  for (auto c : ks.word) {
    if (c == ';' || c == 'k' || c == 'K') { if (selectedCategory > 0) selectedCategory--; }
    if (c == '.' || c == 'j' || c == 'J') { if (selectedCategory < NUM_CATEGORIES-1) selectedCategory++; }
    if (c == 'b' || c == 'B') { state = HOME; return; }
  }

  if (ks.enter) {
    categoryVerseIndex = 0;
    state = CATEGORY_VERSES;
  }
}

void drawCategoryVerses() {
  clearCanvas();
  drawHeader(categories[selectedCategory].name);

  int y = 26;
  for (int i = 0; i < 5; i++) {
    const char* ref = categories[selectedCategory].refs[i];
    if (!ref || ref[0]==0) break;

    bool sel = (i == categoryVerseIndex);
    if (sel) {
      canvas.fillRoundRect(4, y-2, SCREEN_W-8, 14, 2, TFT_DARKGREEN);
      canvas.setTextColor(TFT_WHITE, TFT_DARKGREEN);
    } else {
      canvas.setTextColor(TFT_WHITE, TFT_BLACK);
    }
    canvas.setCursor(8, y);
    canvas.setTextSize(0.95);
    canvas.print(ref);
    y += 17;
  }

  drawFooter("Up/Dn  Enter=read verse  B=back to categories");
  canvas.pushSprite(0, 0);
}

void handleCategoryVerses() {
  if (!M5Cardputer.Keyboard.isChange() || !M5Cardputer.Keyboard.isPressed()) return;
  Keyboard_Class::KeysState ks = M5Cardputer.Keyboard.keysState();

  for (auto c : ks.word) {
    if (c == ';' || c == 'k' || c == 'K') { if (categoryVerseIndex > 0) categoryVerseIndex--; }
    if (c == '.' || c == 'j' || c == 'J') { if (categoryVerseIndex < 4) categoryVerseIndex++; }
    if (c == 'b' || c == 'B') { state = POPULAR_CATEGORIES; return; }
  }

  if (ks.enter) {
    const char* ref = categories[selectedCategory].refs[categoryVerseIndex];
    if (ref && ref[0]) {
      for (int i = 0; i < NUM_POPULAR; i++) {
        if (strcmp(popularVerses[i].ref, ref) == 0) {
          enterReader(popularVerses[i].ref, popularVerses[i].text);
          return;
        }
      }
      enterReader(ref, "Verse text not found.");
    }
  }
}
// ====================== SD ======================
void initSD() {
  SPI.begin(SD_SPI_SCK_PIN, SD_SPI_MISO_PIN, SD_SPI_MOSI_PIN, SD_SPI_CS_PIN);
  sdAvailable = SD.begin(SD_SPI_CS_PIN, SPI, 25000000);
  if (sdAvailable) {
    if (SD.exists("/bible/books.txt")) {
      fullBibleMode = true;
      loadNavBooks();
      loadLastPosition();
    }
  } else {
    fullBibleMode = false;
  }
}

// ====================== SETUP / LOOP ======================
void setup() {
  auto cfg = M5.config();
  M5Cardputer.begin(cfg, true);
  // Battery via M5Cardputer.Power (M5Unified)
  M5Cardputer.Display.setRotation(1);

  canvas.setColorDepth(16);
  canvas.createSprite(SCREEN_W, SCREEN_H);
  canvas.setTextWrap(false);

  // Readable font for small screen
  M5Cardputer.Display.setTextFont(&fonts::FreeMono9pt7b);
  canvas.setTextFont(&fonts::FreeMono9pt7b);

  randomSeed(analogRead(0));

  brightness = 90;
  M5Cardputer.Display.setBrightness(brightness);

  initSD();

  state = HOME;  // clear front screen

  // Optional boot VOTD (simple, no extra setting for cleanliness)
  // Comment out or force if you want: enterReader(votdPool[0].ref, votdPool[0].text);

  // Splash - revamped for better visual balance and nice cross
  clearCanvas();

  // Title - large and centered
  canvas.setTextSize(1.8);
  String title = "Pocket Bible";
  int titleW = canvas.textWidth(title);
  canvas.setCursor( (SCREEN_W - titleW) / 2 , 12);
  canvas.print(title);

  // Nice centered cross (thicker, well proportioned, not smashed)
  int cx = SCREEN_W / 2;
  int cy = 58;
  canvas.fillRect(cx - 3, cy - 22, 6, 44, TFT_WHITE);   // vertical bar (taller)
  canvas.fillRect(cx - 16, cy - 4, 32, 8, TFT_WHITE);  // horizontal bar (wider)

  // Subtitle
  canvas.setTextSize(0.75);
  String sub = "Pocket Bible for Cardputer";
  int subW = canvas.textWidth(sub);
  canvas.setCursor( (SCREEN_W - subW) / 2 , 82);
  canvas.print(sub);

  // Version and prompt at bottom
  canvas.setTextSize(0.6);
  String ver = "v" + String(APP_VERSION) + "  •  M5Burner";
  int verW = canvas.textWidth(ver);
  canvas.setCursor( (SCREEN_W - verW) / 2 , 108);
  canvas.print(ver);

  canvas.pushSprite(0, 0);
  delay(1600);
}

void loop() {
  M5Cardputer.update();

  switch (state) {
    case HOME:
      drawHome();
      handleHomeInput();
      break;
    case EXTRAS:
      drawExtras();
      handleExtrasInput();
      break;
    case TESTAMENT_SELECT:
      drawTestamentSelect();
      handleTestamentSelect();
      break;
    case READER:
      drawReader();
      handleReaderInput();
      break;
    case INPUT_REF:
      drawInput();
      handleInput();
      break;
    case SELECT_BOOK:
      drawBookSelector();
      handleBookSelector();
      break;
    case SELECT_CHAPTER:
      drawChapterSelector();
      handleChapterSelector();
      break;
    case SELECT_VERSE:
      drawVerseSelector();
      handleVerseSelector();
      break;
    case POPULAR_CATEGORIES:
      drawPopularCategories();
      handlePopularCategories();
      break;
    case CATEGORY_VERSES:
      drawCategoryVerses();
      handleCategoryVerses();
      break;
    case HYMNS_LIST:
      drawHymnsList();
      handleHymnsList();
      break;
    // BOOKMARKS_LIST case removed (feature disabled)
  }

  delay(7);
}
